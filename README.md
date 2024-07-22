Summary:

MPU 6050 is used to send gyroscope and accelerometer readings and also used to trigger ESP 32 which is in deep sleep mode.

Once it is triggered,ESP32 will get GPS coordinates from SIM A7672S and send a message to the designated number.
ESP32 ( WIFI ) is connected to the SIM's mobile data.

For continous monitoring , ESP 32 is connected to Google FireBase as it supports real time database modifications.

Moreover, a SOS button is also included , when pressed will do the same process as mentioned above.

