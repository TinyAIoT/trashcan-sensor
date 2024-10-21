#include <Arduino.h>
#include <vl53l8cx.h>
#include <Wire.h>
#include "debug.h"
#include "tof_sensor.h"

#define RESOLUTION VL53L8CX_RESOLUTION_4X4

static VL53L8CX sTof(&Wire, -1, -1);
Status TofSensor::gStatus;
uint32_t TofSensor::gDistance;

void TofSensor::begin() {
  /* send the desired settings to the ToF sensor */
	Wire.begin();
  Wire.setClock(1000000);
  sTof.begin();
  sTof.init();
  sTof.set_resolution(RESOLUTION);
  sTof.set_ranging_frequency_hz(1);
  sTof.set_ranging_mode(VL53L8CX_RANGING_MODE_AUTONOMOUS);
  sTof.set_integration_time_ms(5);
  sTof.set_sharpener_percent(10);
  sTof.set_target_order(VL53L8CX_TARGET_ORDER_CLOSEST);
  sTof.start_ranging();
  DEBUG(Serial.println("VL53L8CX initialized successfully"));
  gStatus = Status::MEASURING;
  gDistance = -1;
  DEBUG(Serial.println("Measuring distance..."));
}

void TofSensor::measure() {
  uint8_t ready;
  VL53L8CX_ResultsData data;
  int32_t weightedSum, sumOfWeights;
  size_t index;

  if (gStatus != Status::MEASURING)
    return;
  /* wait for ranging data */
  sTof.check_data_ready(&ready);
  if (!ready) {
    DEBUG(Serial.println("VL53L8CX not ready yet..."));
    return;
  }
  sTof.get_ranging_data(&data);
  /* compute a weighted sum from the distances on the grid based on confidence levels */
  weightedSum = 0;
  sumOfWeights = 0;
  for (index = 0; index < RESOLUTION * VL53L8CX_NB_TARGET_PER_ZONE; index++) {
    if(data.distance_mm[index]<5000 && data.distance_mm[index]>0) {
      switch (data.target_status[index]) {
        /* prefer distances with status five, as it implies ~100% confidence */
        case 5:
          weightedSum += 2 * data.distance_mm[index];
          sumOfWeights += 2;
          break;
        /* apply no weight to distances with status six and nine, as they imply ~50% confidence */
        case 6:
        case 9:
          weightedSum += data.distance_mm[index];
          sumOfWeights++;
          break;
        /* discard measurements with any other status */
        default:
          break;
      }
    }
  }
  /* check if the intermediate results are valid */
  if (weightedSum < 0 && sumOfWeights <= 0) {
    DEBUG(Serial.println("Distance measurement failed."));
    gStatus = Status::FAILED;
    return;
  }
  DEBUG(Serial.printf("weightedSum is %lu and sumOfWeights is %lu.\n", weightedSum, sumOfWeights));
  /* compute and clamp the final distance */
  weightedSum /= sumOfWeights;
  gDistance = weightedSum > UINT16_MAX ? UINT16_MAX : weightedSum;
  DEBUG(Serial.printf("Distance is %lu mm.\n", gDistance));
  gStatus = Status::SUCCEEDED;
}

void TofSensor::end() {
  /* shut down the ToF sensor */
  sTof.stop_ranging();
  sTof.set_power_mode(VL53L8CX_POWER_MODE_SLEEP);
  sTof.end();
  Wire.end();
}