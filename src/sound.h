#ifndef _SOUND_H_
#define _SOUND_H_


#include <stdint.h>

#define FAIL_NEXT +1
#define FAIL_PREV -1
#define FAIL_RANDOM 0
int start_file(int num, int updown);

void sound_setup();

extern TaskHandle_t audio_task_handle;

bool sound_is_playing();
bool sound_is_gain();
uint32_t sound_current_time();
uint32_t sound_duration();
void sound_pause();
void sound_resume();
void sound_stop();
bool sound_start(char * filepath);


#endif // _SOUND_H_
