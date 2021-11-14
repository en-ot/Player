#ifndef _SOUND_H_
#define _SOUND_H_


#include <stdint.h>

typedef enum
{
    SOUND_STOPPED = 0,
    SOUND_ERROR,
    SOUND_STARTING,
    SOUND_PAUSED,
    SOUND_PLAYING,
} SoundState;
extern SoundState sound_state;

void sound_play_cmd(const char * filename);
void sound_stop_cmd();
void sound_wait();

void sound_setup(QueueHandle_t tag_queue);

extern TaskHandle_t audio_task_handle;

bool sound_is_playing();
bool sound_is_gain();
uint32_t sound_current_time();
uint32_t sound_duration();
void sound_pause();
void sound_resume();


#endif // _SOUND_H_
