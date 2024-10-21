#ifndef DEBUG_H
#define DEBUG_H

#include "settings.h"

#if defined(SETTINGS_DEBUG_MODE) && SETTINGS_DEBUG_MODE != 0
  #define DEBUG(code) code
#else
  #define DEBUG(code)
#endif

#endif