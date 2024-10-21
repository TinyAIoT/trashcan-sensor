#ifndef MQTT_TRANSCEIVER_H
#define MQTT_TRANSCEIVER_H

#include "WiFi.h"   // Use the appropriate WiFi library for your board
#include <ArduinoMqttClient.h>
#include "status.h"

namespace MqttTransceiver {
  extern Status gStatus;
  void begin();
  void transmit();
  void poll();
  inline void end() {}
}

#endif