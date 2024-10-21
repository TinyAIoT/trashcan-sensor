#ifndef BATTERY_H
#define BATTERY_H

#include "status.h"

namespace Battery {
  extern Status gStatus;
  extern uint32_t gVoltage;
  void begin();
  void measure();
  inline void end() {}
}

#endif