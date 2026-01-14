# GSM Security System - Circuit Schematic Guide

## Component List (Proteus)
1. **ARDUINO UNO R3**
2. **SIM900** (GSM Module)
3. **KEYPAD-PHONE** (or generic 4x4 Matrix Keypad)
4. **LM016L** (16x2 LCD)
5. **PCF8574** (I2C Expander for LCD) - *Optional, if using I2C LCD directly, otherwise wire parallel*
   - *Note: Code assumes I2C LCD at address 0x27 using SDA/SCL pins.*
6. **SERVO** (Generic PWM Servo)
7. **LED-GREEN**, **LED-RED**
8. **SOUNDER** (Active Buzzer mode)
9. **COMPIM** (Physical Serial Port or Virtual Serial Port for GSM debugging)

## Pin Connections

### 1. Keypad (4x4)
| Keypad Pin | Arduino Pin |
|------------|-------------|
| Row A      | D4          |
| Row B      | D5          |
| Row C      | D6          |
| Row D      | D7          |
| Col 1      | A0          |
| Col 2      | A1          |
| Col 3      | A2          |
| Col 4      | A3          |

### 2. LCD Display (I2C Mode)
| LCD Pin | Arduino Pin |
|---------|-------------|
| SDA     | A4 (SDA)    |
| SCL     | A5 (SCL)    |
| VCC     | +5V         |
| GND     | GND         |

### 3. Servo Motor (Lock)
| Servo Pin | Arduino Pin |
|-----------|-------------|
| Signal    | D9          |
| VCC       | +5V         |
| GND       | GND         |

### 4. Indicators & Buzzer
| Component | Arduino Pin |
|-----------|-------------|
| Buzzer    | D10         |
| Green LED | D11         |
| Red LED   | D12         |

### 5. GSM Module (SIM900)
| GSM Pin | Arduino Pin | Note |
|---------|-------------|------|
| TX      | D2          | Arduino RX (SoftwareSerial) |
| RX      | D3          | Arduino TX (SoftwareSerial) |
| GND     | GND         | Common ground is critical |

## Proteus Simulation Notes
- **I2C Library**: Ensure you have the `LiquidCrystal_I2C` library installed in your Arduino IDE libraries folder so the hex compiler can find it.
- **GSM Simulation**: The SIM900 component in Proteus usually needs a physical COM port bridge (COMPIM) or a virtual terminal to respond to AT commands. Since SMS is currently deferred/commented out, the GSM module presence is largely validational at this stage.
- **Hex File**: Compile `GSM_Security_System.ino` in Arduino IDE to generate the `.hex` file. Double-click the Arduino implementation in Proteus and load this file in "Program File".
