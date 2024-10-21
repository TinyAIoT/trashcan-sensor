#include <Arduino.h>
#include "battery.h"
#include "debug.h"

#define DIVIDER_PIN A3

static uint32_t sCount;
Status Battery::gStatus;
uint32_t Battery::gVoltage;

void Battery::begin() {
  /* apply the desired settings to the voltage divider's analog GPIO pin */
  pinMode(DIVIDER_PIN, INPUT);
  analogReadResolution(13);
  analogSetPinAttenuation(DIVIDER_PIN, ADC_11db);
  DEBUG(Serial.println("Voltage measure initialized successfully"));
  gStatus = Status::MEASURING;
  gVoltage = 0;
  sCount = 0;
  DEBUG(Serial.println("Measuring voltage..."));
}

void Battery::measure() {
  if (gStatus != Status::MEASURING)
    return;
  /* collect a single voltage measurement */
  gVoltage += analogReadMilliVolts(DIVIDER_PIN);
  sCount++;
  /* compute the battery voltage from fifteen individual measurements */
  if (sCount >= 15) {
    /* multiply the average voltage by two because the voltage divider uses the same resistance twice */
    gVoltage = 2 * (gVoltage / sCount);
    /* clamp the final voltage */
    gVoltage = gVoltage > UINT16_MAX ? UINT16_MAX : gVoltage;
    DEBUG(Serial.printf("Voltage is %lu mV.\n", gVoltage));
    gStatus = Status::SUCCEEDED;
  }
}