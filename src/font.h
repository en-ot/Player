#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

//#define SPIFFS_FONT

//#define FONTSIZE 18
#define FONTSIZE 24


#if (FONTSIZE==18)
#ifdef SPIFFS_FONT
#define FONT_NAME1 "x318"
#else
#define FONT_NAME1 x318
extern const uint8_t x318[];
#endif

#elif (FONTSIZE==24)
#ifdef SPIFFS_FONT
#define FONT_NAME1 "x324"
#else
#define FONT_NAME1 x324
extern const uint8_t x324[];
#endif

#endif // FONTSIZE==


#endif // _FONT_H_
