#include <Arduino.h>
#include "debug.h"

#include "Audio.h"

#include "pinout.h"

#include "globals.h"
#include "prefs.h"
#include "playstack.h"

#include "page_sys.h"   //temp
#include "player.h"

#include "sound.h"

Audio audio;

static QueueHandle_t queue;


static int gain_index = 0;
static bool is_gain = false;
#define GAIN_TXT1 "UserDefinedText: "
#define GAIN_TXT2 "replaygain_track_gain"

static char sound_filename[PATHNAME_MAX_LEN] = "";
static bool need_play_file = false;
bool need_stop = false;
bool need_wait = false;

bool sound_start(char * filepath);
void sound_stop();


SoundState sound_state = SOUND_STOPPED;

//###############################################################
// Audio wrapper
//###############################################################
void playctrl_loop();

TaskHandle_t audio_task_handle;
static void sound_task(void * pvParameters)
{
    while (true)
    {
        audio.loop();
        playctrl_loop();
        vTaskDelay(1);      //5 ok, 7 bad
    }
}


void sound_setup(QueueHandle_t tag_queue)
{
    queue = tag_queue;
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    xTaskCreatePinnedToCore(sound_task, "sound_task", 5000, NULL, 2, &audio_task_handle, SOUND_CORE);
}


bool sound_is_gain()
{
    return is_gain;
}


bool sound_is_playing()
{
    return audio.isRunning();
}


uint32_t sound_current_time()
{
    return audio.getAudioCurrentTime();
}


uint32_t sound_duration()
{
    return audio.getAudioFileDuration();
}


void sound_pause_resume()
{
    audio.pauseResume();
    sound_state = sound_is_playing() ? SOUND_PLAYING : SOUND_PAUSED;
}


void sound_pause()
{
    if (sound_is_playing())
        sound_pause_resume();
}


void sound_resume()
{
    if (!sound_is_playing())
        sound_pause_resume();
}


void sound_stop()
{
    audio.stopSong();
    sound_state = SOUND_STOPPED;
}


bool sound_start(char * filepath)
{
    bool result = audio.connecttoSD(filepath);
    sound_state = result ? SOUND_PLAYING : SOUND_ERROR;
    return result;
}


//###############################################################
// called from other thread
//###############################################################
void sound_play_cmd(const char * filename)
{
    gain_index = 0;
    is_gain = false;
    audio.setReplayGain(1.0);

    sound_state = SOUND_STARTING;
    strncpy(sound_filename, filename, sizeof(sound_filename));
    need_play_file = true;
}


void sound_stop_cmd()
{
    need_stop = true;
}


void sound_wait()
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

void playctrl_loop()
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
        sound_start(sound_filename);
    }

    if (need_stop)
    {
        need_stop = false;
        if (sound_is_playing())
            sound_stop();
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
            gain_index = 2;
        }
        else if (gain_index == 2)
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
                is_gain = true;
            }
            else
            {
                //DEBUG("!sscanf\n");
            }
        }
        gain_index++;
    }
    xQueueSend(queue, info, 0);
}


void audio_id3image(File& file, const size_t pos, const size_t size)
{
    char filename[PATHNAME_MAX_LEN] = "";
    file.getName(filename, sizeof(filename)-1);
    DEBUG("Image %s %d %d\n", filename, pos, size);
}
