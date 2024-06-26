#ifndef LORA_TRANSCEIVER_H
#define LORA_TRANSCEIVER_H

#include "status.h"

namespace LoraTransceiver {
  extern Status gStatus;
  void begin();
  void transmit();
  inline void end() {}
}

#endif