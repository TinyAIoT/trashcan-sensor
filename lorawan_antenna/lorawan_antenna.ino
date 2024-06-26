/***************************************************************************************
 IMPORTANT NOTICE
 ***************************************************************************************
 Please ensure that the the file "project_config/lmic_project_config.h" in your local
 installation of the MCCI LoRaWAN LMIC library has the same definitions as the
 configuration in this repository.
 Furthermore, please enter device-specific information from The Things Network into the
 file "ttn_config_template.txt" and save it as "ttn_config.h".
 ***************************************************************************************/

#include <arduino_lmic.h>
#include <hal/hal.h>
#include "ttn_config.h"

static const u1_t spDeviceEui[TTN_EUI_SIZE] = {TTN_DEVICE_EUI};
static const u1_t spAppKey[TTN_KEY_SIZE] = {TTN_APP_KEY};

void os_getDevEui(u1_t *pBuffer) { memcpy(pBuffer, spDeviceEui, TTN_EUI_SIZE); }
void os_getDevKey(u1_t *pBuffer) { memcpy(pBuffer, spAppKey, TTN_KEY_SIZE); }
void os_getArtEui(u1_t *pBuffer) { memset(pBuffer, 0x00, TTN_EUI_SIZE); }

static const lmic_pinmap sPinMap = {
  .nss  = SS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst  = D0,
  .dio  = {D1, D2, LMIC_UNUSED_PIN}
};

static osjob_t sInitializeJob, sSendJob;

static void initialize(osjob_t *pJob)
{
  LMIC_registerEventCb(&handleEvent, nullptr);
  LMIC_reset();
  LMIC_startJoining();
}

static void send(osjob_t *pJob)
{
  Serial.println("Transmitting...");
  LMIC_setTxData2(1u, (xref2u1_t) "Hi!", 4u, 0u);
}

static void handleEvent(void *pUserData, ev_t event) {
  switch (event) {
    case EV_JOINING:
      Serial.println("Joining...");
      return;
    case EV_JOIN_TXCOMPLETE:
      Serial.println("Join cycle completed.");
      return;
    case EV_JOINED:
      Serial.println("Join successful.");
      LMIC_setLinkCheckMode(0);
      os_setCallback(&sSendJob, &send);
      return;
    case EV_JOIN_FAILED:
      Serial.println("Join failed.");
      return;
    case EV_TXCOMPLETE:
      Serial.println("Transmitted.");
    default:
      return;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(100);
  os_init_ex(&sPinMap);
  os_setCallback(&sInitializeJob, &initialize);
}

void loop() {
  os_runloop_once();
}