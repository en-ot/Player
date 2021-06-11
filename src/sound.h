#ifndef _SOUND_H_
#define _SOUND_H_


#include <stdint.h>

int start_file(int num, int updown);

void sound_setup();
void sound_task();

bool sound_is_playing();
uint32_t sound_current_time();
uint32_t sound_duration();
void sound_pause();
void sound_resume();
void sound_stop();
bool sound_start(char * filepath);


//void sound_pause_resume();


#endif // _SOUND_H_

