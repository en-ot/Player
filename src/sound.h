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

    SoundState state = SOUND_STOPPED;
    void play_cmd(const char * filename);
    void stop_cmd();
    void wait();
    bool is_playing();
    bool is_gain();
    uint32_t current_time();
    uint32_t duration();

    void pause_resume();
    void pause();
    void resume();

    void id3data(const char *info);

private:
    class SoundPrivate * p;

    int _gain_index = 0;
    bool _is_gain = false;

    void stop();
    bool start(char * filepath);

    bool need_play_file = false;
    bool need_stop = false;
    bool need_wait = false;
};

extern Sound * sound;

extern TaskHandle_t audio_task_handle;


#endif // _SOUND_H_
