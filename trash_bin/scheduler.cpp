#include <Arduino.h>
#include "debug.h"
#include "scheduler.h"
#include "settings.h"

#if !defined(SETTINGS_SLEEP_TIME) || SETTINGS_SLEEP_TIME <= 0
  #error "SETTINGS_SLEEP_TIME must be a positive integer"
#endif

#define JOB_POOL_SIZE 2

/* a custom pin map for the LMIC library */
static const lmic_pinmap sPinMap = {
  .nss            = SS,
  .rxtx           = LMIC_UNUSED_PIN,
  .rst            = D0,
  .dio            = {D1, D2, LMIC_UNUSED_PIN},
  .rxtx_rx_active = 0,
  .rssi_cal       = 0,
  .spi_freq       = 0,
  .pConfig        = nullptr
};

/* an array that manages a sequence of jobs */
static osjob_t spJobPool[JOB_POOL_SIZE];
static size_t sJobIndex;

/* a structure that retains the LMIC library's data during deep sleep */
RTC_DATA_ATTR lmic_t gPersistentLmic;
Status Scheduler::gStatus;

/* a function that adjusts the LMIC library's timings */
static void correctDutyCycles() {
#ifdef CFG_LMIC_EU_like
  ostime_t futureTime, correctedTime;
  size_t index;

  futureTime = os_getTime() + sec2osticks(SETTINGS_SLEEP_TIME);
  for (index = 0; index < MAX_BANDS; index++) {
    correctedTime = gPersistentLmic.bands[index].avail - futureTime;
    gPersistentLmic.bands[index].avail = correctedTime < 0 ? 0 : correctedTime;
  }
  correctedTime = gPersistentLmic.globalDutyAvail - futureTime;
  gPersistentLmic.globalDutyAvail = correctedTime < 0 ? 0 : correctedTime;
#else
  #warning "duty cycle correction is only implemented for EU regions"
#endif
}

/* this function is implemented in the transceiver's source code */
void handleEvent(void *pUserData, ev_t event);

void Scheduler::begin() {
  sJobIndex = 0;
  gStatus = Status::SCHEDULING;
	os_init_ex(&sPinMap);
  LMIC_reset();
  LMIC_registerEventCb(&handleEvent, nullptr);
  /* load the LMIC library's data if it was already initialized */
  if (gPersistentLmic.seqnoUp != 0)
    LMIC = gPersistentLmic;
}

void Scheduler::enqueue(osjobcb_t job) {
  /* get the next job from the array and schedule it to execute in ten milliseconds */
  os_setTimedCallback(&spJobPool[sJobIndex], os_getTime() + ms2osticks(10), job);
  sJobIndex = (sJobIndex + 1) % JOB_POOL_SIZE;
}

void Scheduler::schedule() {
  os_runloop_once();
  /* succeed if there are no more pending jobs or transmissions */
  if (!os_queryTimeCriticalJobs(sec2osticks(SETTINGS_SLEEP_TIME)) && LMIC_queryTxReady())
    gStatus = Status::SUCCEEDED;
}

void Scheduler::end() {
  /* save the LMIC library's data and stop all of its activities */
  gPersistentLmic = LMIC;
  LMIC_shutdown();
  /* adjust timings for the next measurement cycle */
  correctDutyCycles();
  /* go to deep sleep */
  DEBUG(Serial.printf("Sleeping for %d s...\n", SETTINGS_SLEEP_TIME));
  DEBUG(delay(250));
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_enable_timer_wakeup(UINT64_C(1000000) * SETTINGS_SLEEP_TIME);
  esp_deep_sleep_start();
}