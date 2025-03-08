#include <Wire.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#define FIREBASE_HOST "your-firebase-url"
#define FIREBASE_AUTH "your-firebase-auth-key"
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"

#define MPU6050_ADDR 0x68
#define SOS_BUTTON_PIN 4

FirebaseData firebaseData;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize Serial2 for SIM A7672S (use correct GPIOs for TX and RX)
    Serial2.begin(115200, SERIAL_8N1, 16, 17);

    // Check MPU6050 connection
    Wire.beginTransmission(MPU6050_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("MPU6050 not detected! Check connections.");
    } else {
        Serial.println("MPU6050 detected.");
        Wire.beginTransmission(MPU6050_ADDR);
        Wire.write(0x6B); // PWR_MGMT_1 register
        Wire.write(0);    // Wake up the sensor
        Wire.endTransmission(true);
    }

    // Initialize WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(" Connected!");

    // Initialize Firebase
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);

    // Set SOS button as input with pull-up resistor
    pinMode(SOS_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
    if (detectFall()) {
        Serial.println("Fall detected!");
        handleEmergency();
    }

    if (digitalRead(SOS_BUTTON_PIN) == LOW) {
        delay(200); // Debounce delay
        if (digitalRead(SOS_BUTTON_PIN) == LOW) { // Confirm button press
            Serial.println("SOS button pressed!");
            handleEmergency();
        }
    }

    delay(500);
}

bool detectFall() {
    int16_t accelX, accelY, accelZ;
    
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 6, true);

    accelX = (Wire.read() << 8 | Wire.read());
    accelY = (Wire.read() << 8 | Wire.read());
    accelZ = (Wire.read() << 8 | Wire.read());

    float accelX_g = accelX / 16384.0;
    float accelY_g = accelY / 16384.0;
    float accelZ_g = accelZ / 16384.0;
    float totalAcceleration = sqrt(accelX_g * accelX_g + accelY_g * accelY_g + accelZ_g * accelZ_g);

    if (totalAcceleration < 0.6) {  // Adjusted free-fall threshold
        delay(100); // Short delay to allow impact

        // Read values again to confirm impact
        Wire.beginTransmission(MPU6050_ADDR);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(MPU6050_ADDR, 6, true);

        accelX = (Wire.read() << 8 | Wire.read());
        accelY = (Wire.read() << 8 | Wire.read());
        accelZ = (Wire.read() << 8 | Wire.read());

        accelX_g = accelX / 16384.0;
        accelY_g = accelY / 16384.0;
        accelZ_g = accelZ / 16384.0;
        totalAcceleration = sqrt(accelX_g * accelX_g + accelY_g * accelY_g + accelZ_g * accelZ_g);

        if (totalAcceleration > 1.5) { // Impact threshold
            return true;
        }
    }
    return false;
}

void handleEmergency() {
    String gpsLocation = fetchGPSCoordinates();
    sendSMS(gpsLocation);
    updateFirebase(gpsLocation);

    // Enable wake-up timer for 3 minutes before entering deep sleep
    esp_sleep_enable_timer_wakeup(3 * 60 * 1000000);
    esp_deep_sleep_start();
}

String fetchGPSCoordinates() {
    Serial.println("Fetching GPS coordinates...");
    Serial2.println("AT+CGNSINF");
    delay(2000); // Ensure the response is received

    String gpsResponse = "";
    while (Serial2.available()) {
        gpsResponse += (char)Serial2.read();
    }
    Serial.println("GPS Response: " + gpsResponse);

    if (!gpsResponse.startsWith("+CGNSINF")) {
        Serial.println("Error: Invalid GPS response.");
        return "Unknown";
    }

    int latStart = gpsResponse.indexOf(",") + 1;
    for (int i = 0; i < 2; i++) {
        latStart = gpsResponse.indexOf(",", latStart) + 1;
    }
    int latEnd = gpsResponse.indexOf(",", latStart);
    int lonStart = latEnd + 1;
    int lonEnd = gpsResponse.indexOf(",", lonStart);

    String latitude = gpsResponse.substring(latStart, latEnd);
    String longitude = gpsResponse.substring(lonStart, lonEnd);

    if (latitude == "0.000000" || longitude == "0.000000" || latitude.length() == 0 || longitude.length() == 0) {
        Serial.println("Error: GPS location invalid or not available.");
        return "Unknown";
    }

    Serial.println("Latitude: " + latitude);
    Serial.println("Longitude: " + longitude);

    return latitude + "," + longitude;
}

void sendSMS(String gpsLocation) {
    Serial.println("Sending SMS...");
    delay(2000); // Give SIM module time to be ready
    Serial2.println("AT+CMGF=1");  // Set SMS mode to text
    delay(1000);
    Serial2.println("AT+CMGS=\"+91XXXXXXXXXX\"");  // Replace with recipient number
    delay(1000);
    Serial2.print("Fall detected! Location: " + gpsLocation);
    Serial2.write(26); // End SMS command
    delay(5000); // Wait for SMS to send
    Serial.println("SMS sent successfully.");
}

void updateFirebase(String gpsLocation) {
    Serial.println("Updating Firebase...");
    if (Firebase.setString(firebaseData, "/fall_detection/location", gpsLocation)) {
        Serial.println("Firebase update successful.");
    } else {
        Serial.println("Firebase update failed: " + firebaseData.errorReason());
    }
}
