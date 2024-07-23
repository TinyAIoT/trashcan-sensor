#include <Arduino.h>
#include <arduino_lmic.h>
#include "battery.h"
#include "debug.h"
#include "lora_transceiver.h"
#include "settings.h"
#include "tof_sensor.h"

#define BUFFER_SIZE 4
#define EUI_SIZE 8
#define KEY_SIZE 16

Status LoraTransceiver::gStatus;

void handleEvent(void *pUserData, ev_t event) {
  switch (event) {
    case EV_JOINING:
      DEBUG(Serial.println("Joining..."));
      return;
    case EV_JOINED:
      DEBUG(Serial.println("Joined."));
      /* prevent occasional signal checks */
      LMIC_setLinkCheckMode(0);
      return;
    case EV_JOIN_FAILED:
      DEBUG(Serial.println("Join failed."));
      LoraTransceiver::gStatus = Status::FAILED;
      return;
    case EV_JOIN_TXCOMPLETE:
      /* only attempt to join the network once */
      if (LMIC.opmode & OP_JOINING) {
        DEBUG(Serial.println("Join timed out."));
        LoraTransceiver::gStatus = Status::FAILED;
      }
      return;
    case EV_TXCOMPLETE:
      if (LoraTransceiver::gStatus == Status::TRANSMITTING) {
        DEBUG(Serial.println("Transmitted."));
        LoraTransceiver::gStatus = Status::SUCCEEDED;
      }
      return;
    default:
      return;
  }
}

/* a callback function required by the LMIC library to get the device EUI */
void os_getDevEui(u1_t *pBuffer) {
  const u1_t pDeviceEui[EUI_SIZE] = {SETTINGS_DEVICE_EUI};
  memcpy(pBuffer, pDeviceEui, EUI_SIZE);
}

/* a callback function required by the LMIC library to get the app key */
void os_getDevKey(u1_t *pBuffer) {
  const u1_t pAppKey[KEY_SIZE] = {SETTINGS_APP_KEY};
  memcpy(pBuffer, pAppKey, KEY_SIZE);
}

/* a callback function required by the LMIC library to get the join EUI */
void os_getArtEui(u1_t *pBuffer) {
  memset(pBuffer, 0x00, EUI_SIZE);
}

void LoraTransceiver::begin() {
  DEBUG(Serial.println("Transmitting..."));
  gStatus = Status::TRANSMITTING;
}

void LoraTransceiver::transmit() {
  u1_t pBuffer[BUFFER_SIZE];
  lmic_tx_error_t error;

  if (!LMIC_queryTxReady() || gStatus != Status::TRANSMITTING)
    return;
  /* encode the measured voltage and distance in four bytes:
   * 1. least significant byte of voltage
   * 2. most significant byte of voltage
   * 3. least significant byte of distance
   * 4. most significant byte of distance
   */
  pBuffer[0] = Battery::gVoltage & 0xFFu;
  pBuffer[1] = (Battery::gVoltage >> 8) & 0xFFu;
  pBuffer[2] = TofSensor::gDistance & 0xFFu;
  pBuffer[3] = (TofSensor::gDistance >> 8) & 0xFFu;
  /* try to transmit the encoded measurements */
  error = LMIC_setTxData2(1, pBuffer, BUFFER_SIZE, 1);
  if (error != LMIC_ERROR_SUCCESS) {
    DEBUG(Serial.printf("Transmission failed. (error: %d)\n", error));
    gStatus = Status::FAILED;
  }
}