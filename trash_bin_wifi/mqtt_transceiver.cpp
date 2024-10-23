#include <Arduino.h>
#include <arduino_lmic.h>
#include <Base64.h>
#include "battery.h"
#include "debug.h"
#include "mqtt_transceiver.h"
#include "settings.h"
#include "tof_sensor.h"

#define BUFFER_SIZE 4
#define KEY_SIZE 16

const char* ssid       = "XXX";
const char* pass   = "XXX";

const char broker[] = "XXX";  // IP address of your MQTT broker
int        port     = 1883;
const char topic[]  = "v3/smart-trashcan@ttn/devices/aiconn-trashcan/up";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

Status MqttTransceiver::gStatus;

void MqttTransceiver::begin() {
  // Connect to WiFi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass, 6);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(2000);
  }

  Serial.println("Connected to the network");
  Serial.println();

  // Connect to MQTT broker
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  gStatus = Status::TRANSMITTING;
}

void MqttTransceiver::transmit() {
  /* encode the measured voltage and distance in four bytes:
   * 1. least significant byte of voltage
   * 2. most significant byte of voltage
   * 3. least significant byte of distance
   * 4. most significant byte of distance
   */
  u1_t pBuffer[BUFFER_SIZE];
  pBuffer[0] = Battery::gVoltage & 0xFFu;
  pBuffer[1] = (Battery::gVoltage >> 8) & 0xFFu;
  pBuffer[2] = TofSensor::gDistance & 0xFFu;
  pBuffer[3] = (TofSensor::gDistance >> 8) & 0xFFu;

  // Buffer for the Base64 encoded result
  char encodedData[9]; // Base64 encoded result needs more space than the input (8 chars + null terminator)

  // Convert to hex string
  int encodedLength = Base64.encode(encodedData, reinterpret_cast<char*>(pBuffer), 4);

  // char result[93]; // 88 + 4 + 1 for null terminator

  // Create JSON-like string
  String payload = "{ \"received_at\": \"2024-10-07T07:55:27.577825120Z\", \"uplink_message\": { \"frm_payload\": \"" + String(encodedData) + "\", \"rx_metadata\": [ { \"rssi\": -67, \"channel_rssi\": -67, \"snr\": 9.5 } ] } }";

  // memcpy(result, "{ \"received_at\": \"2024-10-07T07:55:27.577825120Z\", \"uplink_message\": { \"frm_payload\": \"\", \"rx_metadata\": [ { \"rssi\": -67, \"channel_rssi\": -67, \"snr\": 9.5 } ] } }", 40);
  // memcpy(result + 40, pBuffer, 4);
  // strcpy(result + 40 + 4, "{ \"received_at\": \"2024-10-07T07:55:27.577825120Z\", \"uplink_message\": { \"frm_payload\": \"\", \"rx_metadata\": [ { \"rssi\": -67, \"channel_rssi\": -67, \"snr\": 9.5 } ] } }" + 40);

  // Send message
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.println(payload);

  mqttClient.beginMessage(topic);
  mqttClient.print(payload);
  mqttClient.endMessage();

  Serial.println();

  // Call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();
}

void MqttTransceiver::poll() {
  // Call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();
  MqttTransceiver::gStatus = Status::SUCCEEDED;
}
