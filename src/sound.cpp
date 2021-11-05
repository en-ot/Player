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


static int gain_index = 0;
static bool is_gain = false;
#define GAIN_TXT1 "UserDefinedText: "
#define GAIN_TXT2 "replaygain_track_gain"

int sound_errors=0;

//###############################################################
// Audio wrapper
//###############################################################
void playctrl_loop();

static QueueHandle_t queue;


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
    xTaskCreatePinnedToCore(sound_task, "sound_task", 5000, NULL, 2, &audio_task_handle, 1);
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
}


bool sound_start(char * filepath)
{
    return audio.connecttoSD(filepath);
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

    if (player->need_play_next_dir && player->filecnt())
    {
        player->need_play_next_dir = false;
        audio.stopSong();
        player->set_dir(PLAYING, player->next_dir);
        player->next_file = player->cur_file(PLAYING);
        player->need_play_next_file = true;
    }

    if (player->need_play_next_file && player->filecnt())
    {
        player->need_play_next_file = false;
        start_file(player->next_file, player->next_updown);
        player->filepos = 0;
        prefs_save_delayed(need_save_current_file);
        return;
    }

    if (player->volume != player->volume_old)
    {
        player->volume_old = player->volume;
        audio.setVolume(player->volume);
        return;
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


//###############################################################
int start_file(int num, int updown)
{
    if (!player->filecnt())
    {
        return player->cur_file(PLAYING);
    }

    sound_stop();

    num = clamp1(num, player->filecnt());

    xQueueSend(queue, "Path: ", 0);
    xQueueSend(queue, "File: ", 0);
    xQueueSend(queue, "Band: ", 0);
    xQueueSend(queue, "Artist: ", 0);
    xQueueSend(queue, "Album: ", 0);
    xQueueSend(queue, "Title: ", 0);

    char tmp[QUEUE_MSG_SIZE];
    char filename[PATHNAME_MAX_LEN] = "";
    char dirname[PATHNAME_MAX_LEN] = "";
    char filepath[PATHNAME_MAX_LEN] = "";

    int retry = player->filecnt();
    while (retry)
    {
        num = clamp1(num, player->filecnt());
        DEBUG("Trying to play %d...\n", num);

        int level = playstack_is_instack(num);
        if ((player->filecnt() <= PLAYSTACK_LEVELS) || (level == PLAYSTACK_NOT_IN_STACK) || (updown != FAIL_RANDOM))
        {
            if (!player->set_file(PLAYING, num))
            {
                snprintf(tmp, sizeof(tmp)-1, "File: File %d not found", num);
                xQueueSend(queue, tmp, 0);
                DEBUG("no file %d\n", num);
                return player->cur_file(PLAYING);
            }
            
            //DEBUG("Dir %d, File %d\n", playing->curdir, num);

            if (!player->file_is_dir(PLAYING, num))
            {
                player->file_name(PLAYING, num, filename, sizeof(filename));
                //DEBUG("%s\n", filename);

                int x = player->dir_name(PLAYING, num, dirname, sizeof(dirname));
                //DEBUG("%s\n", dirname);

                strlcpy(filepath, dirname, sizeof(filepath));
                filepath[x++] = '/';
                strlcpy(&filepath[x], filename, sizeof(filepath)-x);
                //DEBUG("%s\n", filepath);
                
                gain_index = 0;
                is_gain = false;
                audio.setReplayGain(1.);
                if (sound_start(filepath))
                    break;
            }
        }
        else
        {
            DEBUG("File is in stack %d\n", level);
        }
 
        sound_errors += 1;
        retry--;
        if (updown == FAIL_RANDOM)
            num = player->file_random();
        else
            num += updown;
    }

    if (!retry)
    {
        snprintf(tmp, sizeof(tmp)-1, "File: retry count 0");
        xQueueSend(queue, tmp, 0);
        DEBUG("retry count\n");
        return 0;
    }
    
    playstack_push(num);

    snprintf(tmp, sizeof(tmp)-1, "Index: %i", num);//, playing->filecnt);
    xQueueSend(queue, tmp, 0);
    
    snprintf(tmp, sizeof(tmp)-1, "Path: %s", dirname);
    xQueueSend(queue, tmp, 0);

    snprintf(tmp, sizeof(tmp)-1, "File: %s", filename);
    xQueueSend(queue, tmp, 0);
    
    return player->cur_file(PLAYING);
}



