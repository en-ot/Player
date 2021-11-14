#include <Arduino.h>
#include "debug.h"

#include "Audio.h"

#include "pinout.h"

#include "globals.h"
//#include "prefs.h"
//#include "playstack.h"

#include "page_sys.h"   //temp
//#include "player.h"

#include "sound.h"

Audio audio;

static QueueHandle_t queue;

Sound * sound = nullptr;

#define GAIN_TXT1 "UserDefinedText: "
#define GAIN_TXT2 "replaygain_track_gain"

static char sound_filename[PATHNAME_MAX_LEN] = "";

bool sound_start(char * filepath);
void sound_stop();


//###############################################################
// Audio wrapper
//###############################################################

class SoundPrivate
{

};


TaskHandle_t audio_task_handle;
static void sound_task(void * pvParameters)
{
    while (true)
    {
        audio.loop();
        sound->loop();
        vTaskDelay(1);      //5 ok, 7 bad
    }
}


Sound::Sound(QueueHandle_t tag_queue)
{
    queue = tag_queue;
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    xTaskCreatePinnedToCore(sound_task, "sound_task", 5000, NULL, 2, &audio_task_handle, SOUND_CORE);
}


bool Sound::is_gain()
{
    return _is_gain;
}


bool Sound::is_playing()
{
    return audio.isRunning();
}


uint32_t Sound::current_time()
{
    return audio.getAudioCurrentTime();
}


uint32_t Sound::duration()
{
    return audio.getAudioFileDuration();
}


void Sound::pause_resume()
{
    audio.pauseResume();
    state = is_playing() ? SOUND_PLAYING : SOUND_PAUSED;
}


void Sound::pause()
{
    if (is_playing())
        pause_resume();
}


void Sound::resume()
{
    if (!is_playing())
        pause_resume();
}


void Sound::stop()
{
    audio.stopSong();
    state = SOUND_STOPPED;
}


bool Sound::start(char * filepath)
{
    bool result = audio.connecttoSD(filepath);
    state = result ? SOUND_PLAYING : SOUND_ERROR;
    return result;
}


//###############################################################
// called from other thread
//###############################################################
void Sound::play_cmd(const char * filename)
{
    _gain_index = 0;
    _is_gain = false;
    audio.setReplayGain(1.0);

    state = SOUND_STARTING;
    strncpy(sound_filename, filename, sizeof(sound_filename));
    need_play_file = true;
}


void Sound::stop_cmd()
{
    need_stop = true;
}


void Sound::wait()
{
    need_wait = true;
    while(need_wait)
    {
        yield();
    }
}

//###############################################################
// Play Control : audio task
//###############################################################
uint32_t t_filepos = 0;
uint32_t t_fileseek = 0;
#define T_FILEPOS_DELAY 500
#define T_FILESEEK_DELAY 500
uint32_t old_duration = 0; 

void Sound::loop()
{
    int duration = audio.getAudioFileDuration();
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

            player->filepos = audio.getAudioCurrentTime();
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
        bool running = audio.isRunning();
        
        if (running) audio.pauseResume();
        //audio.loop();

        if (audio.setAudioPlayPosition(player->filepos))
        {
            player->need_set_file_pos = false;
        }
        //audio.loop();
        if (running) audio.pauseResume();
        //audio.loop();
    }

    if (player->volume != player->volume_old)
    {
        player->volume_old = player->volume;
        audio.setVolume(player->volume);
        return;
    } 

    if (need_play_file)
    {
        need_play_file = false;
        start(sound_filename);
    }

    if (need_stop)
    {
        need_stop = false;
        if (is_playing())
            stop();
    }

    if (need_wait)
    {
        need_wait = false;
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
    sound->id3data(info);
}


void Sound::id3data(const char *info)
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
                audio.setReplayGain(gain);
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
