#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include "player_input.h"

#define CONTROLS_PREFS_SIZE (2 * 8 * sizeof(uint16_t))
void controls_set_prefs(uint8_t * src);
void controls_get_prefs(uint8_t * dst);
void controls_calibrate(int n);

void controls_init(bool (*callback)(PlayerInputType type, int key));
void controls_loop();
bool controls_defaults();
void controls_pause();
void controls_resume();


#endif // _CONTROLS_H_
