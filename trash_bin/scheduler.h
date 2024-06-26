#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arduino_lmic.h>
#include <hal/hal.h>
#include "status.h"

namespace Scheduler {
  extern Status gStatus;
	void begin();
	void enqueue(osjobcb_t job);
  void schedule();
  void end();
}

#endif