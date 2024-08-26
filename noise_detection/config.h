#ifndef CONFIG_H
#define CONFIG_H

// Set value to 0/1 to turn console debug messages off/on
#define CONFIG_DEBUG_MODE 0

// ICS-43434 Microphone Pin Connection
#define I2S_SCK SCL
#define I2S_WS D6
#define I2S_SD SDA

// WiFi Configuration
#define WIFI_SSID "WiFi-Network-Name"           // WiFi that the device should connect to
#define WIFI_PASSWORD "WiFi-Network-Password"   // WiFi Password

// API Configuration
#define API_URL "https://backend.com/update"    // POST address of the TinyAIoT Backend
#define AUTH_BEARER "Bearer Secure-Secret-Key"  // Secure API Token

// Project and Sensor IDs
#define PROJECT_ID "Project-Id"     // Project of the device
#define SENSOR_ID "Sensor-Id"       // Identification of one specific device

#endif // CONFIG_H
