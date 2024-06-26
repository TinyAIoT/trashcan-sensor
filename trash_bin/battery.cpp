#include <Arduino.h>
#include "battery.h"
#include "debug.h"

#define DIVIDER_PIN A3

static uint32_t sCount;
Status Battery::gStatus;
uint32_t Battery::gVoltage;

void Battery::begin() {
  pinMode(DIVIDER_PIN, INPUT);
  analogReadResolution(13);
  analogSetPinAttenuation(DIVIDER_PIN, ADC_11db);
  DEBUG(Serial.println("Measuring voltage..."));
  gStatus = Status::MEASURING;
  gVoltage = 0;
  sCount = 0;
}

void Battery::measure() {
  if (gStatus != Status::MEASURING)
    return;
  gVoltage += analogReadMilliVolts(DIVIDER_PIN);
  sCount++;
  if (sCount >= 15) {
    gVoltage = 2 * (gVoltage / sCount);
    gVoltage = gVoltage > UINT16_MAX ? UINT16_MAX : gVoltage;
    DEBUG(Serial.printf("Voltage is %lu mV.\n", gVoltage));
    gStatus = Status::SUCCEEDED;
  }
}