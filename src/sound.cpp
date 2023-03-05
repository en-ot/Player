#include <Arduino.h>

#include "debug.h"
#include "pinout.h"
#include "globals.h"

#include "page_sys.h"   //temp

#include "Audio.h"

#include "sound.h"

#define GAIN_TXT1 "UserDefinedText: "
#define GAIN_TXT2 "replaygain_track_gain"


//###############################################################
// Audio wrapper
//###############################################################

class SoundPrivate
{
public:
    Audio * audio;

    void loop();

    bool _start(const char * filepath);
    void _stop();
    void _wait();

    bool _need_play_file = false;
    bool _need_stop = false;
    bool _need_wait = false;

    const char * _filename;

    SoundState _state = SOUND_STOPPED;

    bool _is_gain = false;
    int _gain_index = 0;
    bool is_gain();

    void pause_resume();

    void id3data(const char *info);

    TaskHandle_t audio_task_handle;

    QueueHandle_t queue;
};

static SoundPrivate * priv = nullptr;
static Sound * sound;


static void sound_task(void * pvParameters)
{
    while (true)
    {
        priv->audio->loop();
        if (sound)
            sound->loop();
        vTaskDelay(1);      //5 ok, 7 bad
    }
}


static Audio audio1;

Sound::Sound(QueueHandle_t tag_queue)
{
    p = new SoundPrivate;
    priv = p;
    sound = this;
    p->queue = tag_queue;
    
    //p->audio = new Audio;
    p->audio = &audio1;
    p->audio->setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

    xTaskCreatePinnedToCore(sound_task, "sound_task", 5000, NULL, 2, &p->audio_task_handle, SOUND_CORE);
}


TaskHandle_t Sound::handle()
{
    return p->audio_task_handle;
}


bool Sound::is_gain()
{
    return p->_is_gain;
}


uint32_t Sound::current_time()
{
    return p->audio->getAudioCurrentTime();
}


uint32_t Sound::duration()
{
    return p->audio->getAudioFileDuration();
}


void SoundPrivate::pause_resume()
{
    audio->pauseResume();
    _state = audio->isRunning() ? SOUND_PLAYING : SOUND_PAUSED;
}


void Sound::pause()
{
    if (p->_state == SOUND_PLAYING)
        p->pause_resume();
}


void Sound::resume()
{
    if (p->_state != SOUND_PLAYING)
        p->pause_resume();
}


void SoundPrivate::_stop()
{
    audio->stopSong();
    _state = SOUND_STOPPED;
}


bool SoundPrivate::_start(const char * filepath)
{
    bool result = audio->connecttoSD(filepath);
    _state = result ? SOUND_PLAYING : SOUND_ERROR;
    return result;
}


//###############################################################
// called from other thread
//###############################################################
void Sound::play(const char * filename)
{
    p->_gain_index = 0;
    p->_is_gain = false;
    p->audio->setReplayGain(1.0);

    p->_filename = filename;
    p->_state = SOUND_STARTING;
    p->_need_play_file = true;

    p->_wait();
}


void Sound::stop()
{
    p->_need_stop = true;
    p->_wait();
}


int Sound::state()
{
    return p->_state;
}


void SoundPrivate::_wait()
{
    _need_wait = true;
    while(_need_wait)
    {
        yield();
    }
}


//###############################################################
// Play Control : audio task
//###############################################################
void Sound::loop()
{
    static uint32_t t_filepos = 0;
    static uint32_t t_fileseek = 0;
    #define T_FILEPOS_DELAY 500
    #define T_FILESEEK_DELAY 500
    static uint32_t old_duration = 0;

    int duration = p->audio->getAudioFileDuration();
    uint32_t t = millis();

    if ((duration != old_duration) || (duration == 0))
    {
        old_duration = duration;
        t_filepos = t;
    }

    if (player->file_seek_by)
    {
        if ((int32_t)(t - t_fileseek) > T_FILESEEK_DELAY)
        {
            t_fileseek = t;

            player->filepos = p->audio->getAudioCurrentTime();
            DEBUG("seek by %d\n", player->file_seek_by);
            int newpos = player->filepos + player->file_seek_by;
            player->file_seek_by = 0;
            if (newpos < 0)     newpos = 0;
            if (newpos > duration) newpos = duration;

            if (newpos == player->filepos)
                return;

            player->filepos = newpos;
            player->need_set_file_pos = true;
            return;
        }
    }

    if (player->need_set_file_pos && ((int32_t)(t - t_filepos) > T_FILEPOS_DELAY))
    {
        bool running = p->audio->isRunning();
        
        if (running) p->audio->pauseResume();
        //audio.loop();

        DEBUG("trying to set file pos %d...", player->filepos);
        if (p->audio->setAudioPlayPosition(player->filepos))
        {
            DEBUG("OK");
            player->need_set_file_pos = false;
        }
        DEBUG("\n");
        
        //audio.loop();
        if (running) p->audio->pauseResume();
        //audio.loop();
    }

    if (player->volume != player->volume_old)
    {
        player->volume_old = player->volume;
        p->audio->setVolume(player->volume);
        return;
    } 

    p->loop();
}

void SoundPrivate::loop()
{
    bool wait_flag = _need_wait;

    if (_need_play_file)
    {
        _need_play_file = false;
        _start(_filename);
        _filename = nullptr;
    }

    if (_need_stop)
    {
        _need_stop = false;
        if (_state == SOUND_PLAYING)
            _stop();
    }

    if (wait_flag)
    {
        _need_wait = false;
    }
}


void audio_eof_mp3(const char *info)      //end of file
{
    DEBUG("eof_mp3     %s\n", info);
    player->play_file_next();
}


void audio_error_mp3(const char *info) 
{
    DEBUG("error_mp3   %s\n", info);
    sys.read_error = true;
}


void audio_info(const char *info) 
{
    DEBUG("info        %s\n", info); 
}


void audio_id3data(const char *info)  //id3 metadata
{
    if (priv)
        priv->id3data(info);
}


void SoundPrivate::id3data(const char *info)
{
    //UserDefinedText: replaygain_track_gain
    //DEBUG("id3 \"%s\"\n", info);
    int l1 = sizeof(GAIN_TXT1)-1;
    if (!strncmp(info, GAIN_TXT1, l1))
    {
        //DEBUG("%d %s\n", gain_index, &info[l1]);
        int l2 = sizeof(GAIN_TXT2)-1;

        int i = 0;
        if (!strncmp(&info[l1], GAIN_TXT2, l2))
        {
            i = l1+l2;
            _gain_index = 2;
        }
        else if (_gain_index == 2)
        {
            i = l1;
        }
        
        if (i)
        {
            //DEBUG("Replay gain: %s\n", &info[i]);
            float gain = 0;
            if (sscanf(&info[i], "%f", &gain))
            {
                //DEBUG("Replay gain %f\n", gain);
                gain = pow10f(-0.05*gain);
                DEBUG("Replay gain %f\n", gain);
                audio->setReplayGain(gain);
                _is_gain = true;
            }
            else
            {
                //DEBUG("!sscanf\n");
            }
        }
        _gain_index++;
    }
    xQueueSend(queue, info, 0);
}


void audio_id3image(File& file, const size_t pos, const size_t size)
{
    char filename[PATHNAME_MAX_LEN] = "";
    file.getName(filename, sizeof(filename)-1);
    DEBUG("Image %s %d [%d]\n", filename, pos, size);
}
