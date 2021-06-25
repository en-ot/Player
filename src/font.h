#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

//#define FONTSIZE 18
#define FONTSIZE 24


#if (FONTSIZE==18)
#define FONT_NAME1 x318
extern const uint8_t x318[];

#elif (FONTSIZE==24)
#define FONT_NAME1 x324
extern const uint8_t x324[];

#endif // FONTSIZE==


#endif // _FONT_H_
