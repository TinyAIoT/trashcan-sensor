#include "battery.h"
#include "debug.h"
#include "lora_transceiver.h"
#include "scheduler.h"
#include "tof_sensor.h"

static void transmit(osjob_t *pJob) {
  LoraTransceiver::transmit();
  if (LoraTransceiver::gStatus == Status::TRANSMITTING) {
    Scheduler::enqueue(&transmit);
    return;
  }
  LoraTransceiver::end();
}

static void measure(osjob_t *pJob) {
  Battery::measure();
  TofSensor::measure();
  if (Battery::gStatus == Status::MEASURING || TofSensor::gStatus == Status::MEASURING) {
    Scheduler::enqueue(&measure);
    return;
  }
  Battery::end();
  TofSensor::end();
  if (Battery::gStatus == Status::FAILED || TofSensor::gStatus == Status::FAILED)
    return;
  LoraTransceiver::begin();
  Scheduler::enqueue(&transmit);
}

static void initialize(osjob_t *pJob) {
  Battery::begin();
  TofSensor::begin();
  Scheduler::enqueue(&measure);
}

void setup() {
  DEBUG(Serial.begin(115200));
  Scheduler::begin();
  Scheduler::enqueue(&initialize);
}

void loop() {
  Scheduler::schedule();
  if (Scheduler::gStatus != Status::SCHEDULING)
    Scheduler::end();
}