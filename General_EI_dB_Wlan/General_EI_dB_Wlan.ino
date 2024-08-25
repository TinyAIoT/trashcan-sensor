#include "wifi_manager.h"
#include "microphone.h"
#include "config.h"
#include "debug.h"

void setup() {
    DEBUG({
        Serial.begin(115200);
        while (!Serial);
        Serial.println("Noise prediction and dBFS level over Wlan");
    });

    // Connect to WiFi
    wifi_manager::connect();

    // Initialize the microphone (including starting inference)
    if (!microphone::initialize()) {
        DEBUG(Serial.println("Failed to initialize the microphone"));
        return;
    }

    DEBUG(microphone::printInferenceSettings());
}

void loop() {
    microphone::InferenceResult inferenceResult = microphone::runFullInference();

    if (!inferenceResult.success) {
        DEBUG(Serial.println("ERR: Failed to run inference"));
        return;
    }

    // Send dBFS and ML prediction over Wi-Fi
    wifi_manager::sendData(inferenceResult.dbfs, inferenceResult.class1Percentage);
}
