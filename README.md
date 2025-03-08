#IoT-Based Fall Detection and Emergency Alert System Using ESP32 and MPU6050
Summary:
Real-time monitoring and emergency assistance for elderly and at-risk individuals.

This project is an ESP32-based fall detection and emergency alert system using the MPU6050 accelerometer, SIM A7672S module for SMS alerts, and Firebase for remote monitoring.

## Features
- **Fall Detection:** Detects falls based on acceleration threshold values.
- **SOS Button:** Allows manual emergency alerts.
- **GPS Integration:** Fetches real-time location using SIM A7672S.
- **SMS Alert:** Sends an emergency message with GPS location.
- **Firebase Update:** Stores the alert and location data in Firebase.
- **Power Optimization:** Enters deep sleep mode after sending alerts to conserve power.

## Components Required
- ESP32
- MPU6050 Accelerometer
- SIM A7672S Module (or similar GSM module with GPS support)
- Push Button (for SOS trigger)
- Connecting Wires

## How It Works
1. **Initialization:**
   - Connects to WiFi.
   - Initializes the MPU6050 sensor.
   - Configures the SIM module for GPS and SMS functionality.
   - Sets up the SOS button as an input with a pull-up resistor.

2. **Fall Detection:**
   - Reads acceleration data from the MPU6050.
   - Detects free-fall if total acceleration drops below 0.6g.
   - Confirms impact with a second acceleration check (threshold >1.5g).
   - If a fall is detected, triggers emergency handling.

3. **SOS Button Trigger:**
   - If the button is pressed, a 200ms debounce delay is applied.
   - If still pressed, an emergency is triggered.

4. **Handling Emergency:**
   - Fetches GPS coordinates using the SIM module.
   - Sends an SMS alert with the location.
   - Updates Firebase with the fall event and GPS data.
   - Enters deep sleep mode for 3 minutes to conserve power.

## Circuit Connections
### MPU6050 to ESP32
| MPU6050 Pin | ESP32 Pin |
|------------|-----------|
| VCC        | 3.3V      |
| GND        | GND       |
| SDA        | GPIO21    |
| SCL        | GPIO22    |

### SIM A7672S to ESP32
| SIM A7672S Pin | ESP32 Pin |
|---------------|-----------|
| VCC           | 5V        |
| GND           | GND       |
| RXD           | GPIO16    |
| TXD           | GPIO17    |

### SOS Button
| SOS Button Pin | ESP32 Pin |
|---------------|-----------|
| One End       | GPIO4     |
| Other End     | GND       |


## Expected Output
- When a fall is detected or the SOS button is pressed:
  - An SMS alert is sent with GPS location.
  - Firebase is updated with location data.
  - ESP32 enters deep sleep mode.
- Serial Monitor will display:
  - MPU6050 initialization status.
  - WiFi connection status.
  - GPS data response.
  - Confirmation of SMS and Firebase update.
 
    
