#ifndef _CONTROLS_H_
#define _CONTROLS_H_

void controls_init();
void controls_loop();
bool controls_defaults();
void controls_pause();
void controls_resume();

bool files_goto_curfile();
bool dirs_goto_curdir();
bool fav_goto_curfav();

bool play_file_next();
int file_random();

#endif // _CONTROLS_H_
