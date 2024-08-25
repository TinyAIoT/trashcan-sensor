#ifndef CONFIG_H
#define CONFIG_H

// Set to 0/1 to set debug messages off/on
#define CONFIG_DEBUG_MODE 0

// ICS-43434 Microphone Pin Connection
#define I2S_SCK SCL
#define I2S_WS D6
#define I2S_SD SDA

// WiFi Configuration
#define WIFI_SSID "FRITZ!Box 7530 PU"
#define WIFI_PASSWORD "83383527870040966533"

// API Configuration
#define API_URL "http://192.168.178.69:5000/update"
#define AUTH_BEARER "Bearer test123"

// Project and Sensor IDs
#define PROJECT_ID "Project123"
#define SENSOR_ID "SensorABC"

#endif // CONFIG_H
