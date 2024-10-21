#include "battery.h"
#include "debug.h"
#include "mqtt_transceiver.h"
#include "tof_sensor.h"

static void end() {
  /* go to deep sleep */
  DEBUG(Serial.printf("Sleeping for %d ms...\n", SETTINGS_SLEEP_TIME));
  DEBUG(delay(1000));
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_enable_timer_wakeup(UINT64_C(1000) * SETTINGS_SLEEP_TIME);
  esp_deep_sleep_start();
}

static void measure() {
  TofSensor::measure();
  while (TofSensor::gStatus == Status::MEASURING) {
    delay(50);
    TofSensor::measure();
  }
  TofSensor::end();
  Battery::measure();
  while (Battery::gStatus == Status::MEASURING) {
    delay(50);
    Battery::measure();
  }
  Battery::end();
  if (Battery::gStatus == Status::FAILED || TofSensor::gStatus == Status::FAILED)
    return;
}

void setup() {
  DEBUG(Serial.begin(115200));
  DEBUG(delay(1000));
  Battery::begin();
  TofSensor::begin();

  measure();

  MqttTransceiver::begin();
  MqttTransceiver::transmit();
  while (MqttTransceiver::gStatus != Status::SUCCEEDED) {
    MqttTransceiver::poll();
    delay(50);
  }
  MqttTransceiver::end();
  end();
}

void loop() {
}