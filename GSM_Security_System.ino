/*
  GSM-Based Access Control & Intrusion Alert System
  
  Features:
  - 4x4 Keypad for PIN entry
  - 16x2 I2C LCD for status display
  - Servo motor for door lock mechanism
  - Failed attempt counter & lockout mode
  - Admin reset capability
  
  PIN Mapping:
  - Keypad Rows (R1-R4): 4, 5, 6, 7
  - Keypad Cols (C1-C4): A0, A1, A2, A3
  - LCD: SDA(A4), SCL(A5)
  - Servo: 9
  - Buzzer: 10
  - LEDs: 11 (Green), 12 (Red)
  - GSM: 2 (RX), 3 (TX)
*/

#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// --- Configuration ---
#define PASS_LEN 5          // PIN length (4 digits + null terminator)
#define MAX_FAILED_ATTEMPTS 3
#define LOCKOUT_TIME 30000  // 30 seconds in milliseconds

// --- PIN Definitions ---
const int SERVO_PIN = 9;
const int BUZZER_PIN = 10;
const int GREEN_LED = 11;
const int RED_LED = 12;
const int GSM_RX = 2;
const int GSM_TX = 3;

// --- Objects ---
Servo doorLock;
SoftwareSerial sim900(GSM_RX, GSM_TX); 
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display. verify address if not working (0x3F is active)

// --- Keypad Setup ---
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {4, 5, 6, 7}; 
byte colPins[COLS] = {A0, A1, A2, A3}; 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// --- State Machine ---
enum SystemState {
  IDLE,
  AUTHENTICATING,
  ACCESS_GRANTED,
  ACCESS_DENIED,
  LOCKOUT
};

SystemState currentState = IDLE;

// --- Global Variables ---
char enteredPass[PASS_LEN];
char masterPass[PASS_LEN] = "1234";
char adminPass[PASS_LEN] = "9999";
byte dataCount = 0;
int failedAttempts = 0;
unsigned long lockoutStartTime = 0;

void setup() {
  Serial.begin(9600);
  sim900.begin(9600); // Standard baud rate for SIM900
  
  // Pin Setup
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  // Actuator Setup
  doorLock.attach(SERVO_PIN);
  doorLock.write(0); // Locked position
  
  // LCD Setup
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Initial State
  resetSystem();
}

void loop() {
  switch (currentState) {
    case IDLE:
      handleIdle();
      break;
    case AUTHENTICATING:
      // State handled by input logic
      break;
    case ACCESS_GRANTED:
      handleAccessGranted();
      break;
    case ACCESS_DENIED:
      handleAccessDenied();
      break;
    case LOCKOUT:
      handleLockout();
      break;
  }
}

// --- State Handlers ---

void handleIdle() {
  lcd.setCursor(0, 0);
  lcd.print("Security System ");
  lcd.setCursor(0, 1);
  lcd.print("Enter PIN:      ");
  
  // Display masked input
  for(int i=0; i<dataCount; i++){
    lcd.setCursor(11+i, 1);
    lcd.print("*");
  }
  
  char customKey = customKeypad.getKey();
  
  if (customKey) {
    // Beep on keypress
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
    
    if (customKey == 'C') { // 'C' to Clear
      clearData();
      return;
    }
    
    if (customKey == '#') { // '#' to Enter
      if(dataCount > 0){
        currentState = AUTHENTICATING;
        verifyPassword();
      }
      return;
    }

    if (dataCount < PASS_LEN - 1) {
      enteredPass[dataCount] = customKey;
      dataCount++;
    }
  }
}

void verifyPassword() {
  lcd.clear();
  lcd.print("Verifying...");
  delay(1000); // Artificial delay for UX
  
  bool isMaster = (strcmp(enteredPass, masterPass) == 0);
  bool isAdmin = (strcmp(enteredPass, adminPass) == 0);
  
  if (isMaster) {
    currentState = ACCESS_GRANTED;
  } else if (isAdmin) {
    lcd.clear();
    lcd.print("ADMIN RESET");
    delay(2000);
    failedAttempts = 0;
    resetSystem();
  } else {
    currentState = ACCESS_DENIED;
  }
}

void handleAccessGranted() {
  lcd.clear();
  lcd.print("ACCESS GRANTED");
  lcd.setCursor(0, 1);
  lcd.print("Door Unlocked");
  
  digitalWrite(GREEN_LED, HIGH);
  doorLock.write(180); // Open
  
  // Success beep
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  
  delay(3000); // Keep door open for 3 seconds
  
  // Reset
  doorLock.write(0); // Close
  digitalWrite(GREEN_LED, LOW);
  failedAttempts = 0; // Reset counter on success
  resetSystem();
}

void handleAccessDenied() {
  failedAttempts++;
  
  lcd.clear();
  lcd.print("ACCESS DENIED");
  digitalWrite(RED_LED, HIGH);
  
  // Error beep
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED, LOW);
  
  lcd.setCursor(0, 1);
  lcd.print("Attempts: ");
  lcd.print(failedAttempts);
  lcd.print("/");
  lcd.print(MAX_FAILED_ATTEMPTS);
  delay(2000);
  
  if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
    // Trigger Security Alert
    sendSecurityAlert(); 
    currentState = LOCKOUT;
    lockoutStartTime = millis();
  } else {
    resetSystem();
  }
}

void handleLockout() {
  lcd.clear();
  lcd.print("SYSTEM LOCKED");
  
  unsigned long timePassed = millis() - lockoutStartTime;
  int remaining = (LOCKOUT_TIME - timePassed) / 1000;
  
  lcd.setCursor(0, 1);
  lcd.print("Wait: ");
  lcd.print(remaining);
  lcd.print("s");
  
  digitalWrite(RED_LED, HIGH); // Constant red light during lockout
  
  // Check if admin reset
  char customKey = customKeypad.getKey();
  if(customKey == 'A') { // 'A' key during lockout could be a "panic" or hidden reset trigger if needed
     // For now, simply blocking strictly
  }

  if (timePassed >= LOCKOUT_TIME) {
     digitalWrite(RED_LED, LOW);
     failedAttempts = 0; // Reset counter after penalty ?? Or keep logic strictly?
     // Common pattern: reset counter after penalty so user can try again
     resetSystem();
  }
  
  delay(100); // Update refresh rate
}

void sendSecurityAlert() {
  lcd.clear();
  lcd.print("! SECURITY ALERT !");
  lcd.setCursor(0, 1);
  lcd.print("Sending SMS...");
  
  // SMS Functionality DEFERRED per user request
  /*
  sim900.print("AT+CMGF=1\r"); 
  delay(100);
  sim900.println("AT+CMGS=\"+919229285100\""); 
  delay(100);
  sim900.println("SECURITY WARNING: Multiple failed web access attempts detected!"); 
  delay(100);
  sim900.println((char)26); 
  delay(1000);
  */
  
  Serial.println("ALERT TRIGGERED: SMS functionality is currently disabled.");
  delay(2000);
}

void clearData() {
  while (dataCount != 0) {
    enteredPass[dataCount--] = 0;
  }
  // Also clear the first char which the loop might miss if logic is off, safely reset
  enteredPass[0] = 0; 
  dataCount = 0;
}

void resetSystem() {
  clearData();
  currentState = IDLE;
  lcd.clear();
}
