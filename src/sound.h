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


class Sound
{
public:
    Sound(QueueHandle_t tag_queue);

    void loop();

    int state();
    void play(const char * filename);
    void stop();
    void pause();
    void resume();

    bool is_gain();
    uint32_t current_time();
    uint32_t duration();

    TaskHandle_t handle();

private:
    class SoundPrivate * p;
};


// extern TaskHandle_t audio_task_handle;


#endif // _SOUND_H_
