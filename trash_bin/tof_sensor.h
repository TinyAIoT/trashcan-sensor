#ifndef TOF_SENSOR_H
#define TOF_SENSOR_H

#include "status.h"

namespace TofSensor {
  extern Status gStatus;
  extern uint32_t gDistance;
	void begin();
  void measure();
  void end();
}

#endif