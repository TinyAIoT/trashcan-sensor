#include <Arduino.h>
#include <vl53l8cx_class.h>
#include <Wire.h>
#include "debug.h"
#include "tof_sensor.h"

#define RESOLUTION VL53L8CX_RESOLUTION_4X4

static VL53L8CX sTof(&Wire, -1, -1);
Status TofSensor::gStatus;
uint32_t TofSensor::gDistance;

void TofSensor::begin() {
	Wire.begin();
  Wire.setClock(1000000);
  sTof.begin();
  sTof.init_sensor();
  sTof.vl53l8cx_set_resolution(RESOLUTION);
  sTof.vl53l8cx_set_ranging_frequency_hz(1);
  sTof.vl53l8cx_set_ranging_mode(VL53L8CX_RANGING_MODE_AUTONOMOUS);
  sTof.vl53l8cx_set_integration_time_ms(5);
  sTof.vl53l8cx_set_sharpener_percent(10);
  sTof.vl53l8cx_set_target_order(VL53L8CX_TARGET_ORDER_CLOSEST);
  sTof.vl53l8cx_start_ranging();
  DEBUG(Serial.println("Measuring distance..."));
  gStatus = Status::MEASURING;
  gDistance = -1;
}

void TofSensor::measure() {
  uint8_t ready;
  VL53L8CX_ResultsData data;
  int32_t weightedSum, sumOfWeights;
  size_t index;

  if (gStatus != Status::MEASURING)
    return;
  sTof.vl53l8cx_check_data_ready(&ready);
  if (!ready)
    return;
  sTof.vl53l8cx_get_ranging_data(&data);
  weightedSum = 0;
  sumOfWeights = 0;
  for (index = 0; index < RESOLUTION * VL53L8CX_NB_TARGET_PER_ZONE; index++) {
    switch (data.target_status[index]) {
      case 5:
        weightedSum += 2 * data.distance_mm[index];
        sumOfWeights += 2;
        break;
      case 6:
      case 9:
        weightedSum += data.distance_mm[index];
        sumOfWeights++;
        break;
      default:
        break;
    }
  }
  if (weightedSum < 0 || sumOfWeights <= 0) {
    gStatus = Status::FAILED;
    return;
  }
  weightedSum /= sumOfWeights;
  gDistance = weightedSum > UINT16_MAX ? UINT16_MAX : weightedSum;
  DEBUG(Serial.printf("Distance is %lu mm.\n", gDistance));
  gStatus = Status::SUCCEEDED;
}

void TofSensor::end() {
  sTof.vl53l8cx_stop_ranging();
  sTof.vl53l8cx_set_power_mode(VL53L8CX_POWER_MODE_SLEEP);
  sTof.end();
  Wire.end();
}