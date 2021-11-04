#ifndef _CONTROLS_H_
#define _CONTROLS_H_

void controls_init();
void controls_loop();
bool controls_defaults();
void controls_pause();
void controls_resume();

#define CONTROLS_PREFS_SIZE (2 * 8 * sizeof(uint16_t))
void controls_set_prefs(uint8_t * src);
void controls_get_prefs(uint8_t * dst);
void controls_calibrate(int n);


#endif // _CONTROLS_H_
