#include "font.h"

#include <Arduino.h>

#if (FONTSIZE==18)
#ifdef SPIFFS_FONT
#else
#include "Fonts/x318.h"
#endif

#elif (FONTSIZE==24)
#ifdef SPIFFS_FONT
#else
#include "Fonts/x324.h"
#endif

#endif
