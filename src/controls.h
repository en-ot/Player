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

bool files_goto_curfile();
bool dirs_goto_curdir();
bool fav_goto_curfav();

bool play_file_next();
int file_random();


#endif // _CONTROLS_H_
