# GSM-Based Access Control System

A secure, Arduino-based access control system demonstration designed for Proteus simulation.

## Features
- **Secure Authentication**: 4-10 digit keypad entry (Default: `1234`).
- **Visual Interface**: 16x2 LCD displaying system status and masked PIN usage.
- **Door Control**: Servo motor actuation upon successful entry.
- **Intrusion Detection**: Tracks failed attempts; triggers lockout after 3 failures.
- **Admin Reset**: Emergency reset code (Default: `9999`) to clear lockouts.

## Setup Instructions

### 1. Prerequisites
- **Arduino IDE**: For compiling the code.
- **Proteus 8**: For circuit simulation.
- **Libraries**:
  - `Keypad` by Mark Stanley, Alexander Brevig
  - `LiquidCrystal_I2C` by Frank de Brabander
  - `Servo` (Built-in)
  - `SoftwareSerial` (Built-in)

### 2. Compilation
1. Open `GSM_Security_System.ino` in Arduino IDE.
2. Select Board: **Arduino Uno**.
3. Verify/Compile to generate the `.hex` file.

### 3. Simulation
1. Build the circuit in Proteus following `circuit_schematic.md`.
2. Load the generated `.hex` file into the Arduino component.
3. Run the simulation.

## Usage Guide
- **Normal Operation**: Screen shows "Enter PIN". Type `1234` then `#`. Door unlocks.
- **Clear Entry**: Press `C` to clear mistyped digits.
- **Intruder Alert**: Enter wrong code 3 times. System enters "LOCKED" state for 30 seconds.
- **Admin Override**: Type `9999` then `#` to force reset system logic.

## Project Structure
- `GSM_Security_System.ino`: Main firmware.
- `circuit_schematic.md`: Wiring reference.
- `working_final.ino`: Legacy/Backup code.
