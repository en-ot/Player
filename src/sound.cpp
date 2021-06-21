#include <Arduino.h>
#include "debug.h"

#include "Audio.h"

#include "globals.h"
#include "prefs.h"
#include "playstack.h"
#include "controls.h"

#include "sound.h"

Audio audio;


static int gain_index = 0;
static bool is_gain = false;
#define GAIN_TXT1 "UserDefinedText: "
#define GAIN_TXT2 "replaygain_track_gain"


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


void sound_setup()
{
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
unsigned long t_filepos = 0;
unsigned long t_fileseek = 0;
#define T_FILEPOS_DELAY 500
#define T_FILESEEK_DELAY 500
uint32_t old_duration = 0; 

void playctrl_loop()
{
    int duration = audio.getAudioFileDuration();
    unsigned long t = millis();

    if ((duration != old_duration) || (duration == 0))
    {
        old_duration = duration;
        t_filepos = t;
    }

    if (file_seek_by)
    {
        if (t - t_fileseek > T_FILESEEK_DELAY)
        {
            t_fileseek = t;

            filepos = audio.getAudioCurrentTime();
            DEBUG("seek by %d\n", file_seek_by);
            int newpos = filepos + file_seek_by;
            file_seek_by = 0;
            if (newpos < 0)     newpos = 0;
            if (newpos > duration) newpos = duration;

            if (newpos == filepos)
                return;

            filepos = newpos;
            need_set_file_pos = true;
            return;
        }
    }

    if (need_set_file_pos && audio.isRunning() && (t - t_filepos > T_FILEPOS_DELAY))
    {
        //audio.pauseResume();
        //audio.loop();
        if (audio.setAudioPlayPosition(filepos))
        {
            need_set_file_pos = false;
        }
        //audio.loop();
        //audio.pauseResume();
        //audio.loop();
    }

    if (need_play_next_dir && fc->filecnt)
    {
        need_play_next_dir = false;
        audio.stopSong();
        fc->find_dir(next_dir);
        next_file = fc->curfile;
        need_play_next_file = true;
    }

    if (need_play_next_file && fc->filecnt)
    {
        need_play_next_file = false;
        start_file(next_file, next_updown);
        filepos = 0;
        prefs_save_delayed(need_save_current_file);
        return;
    }

    if (volume != volume_old)
    {
        volume_old = volume;
        audio.setVolume(volume);
        return;
    } 
}


void audio_eof_mp3(const char *info)      //end of file
{
    DEBUG("eof_mp3     %s\n", info);
    play_file_next();
}


void audio_error_mp3(const char *info) 
{
    DEBUG("error_mp3   %s\n", info);
    read_error = true;
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
    xQueueSend(tag_queue, info, 0);
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
    if (!fc->filecnt)
    {
        return fc->curfile;
    }

    sound_stop();

    num = clamp1(num, fc->filecnt);

    xQueueSend(tag_queue, "Path: ", 0);
    xQueueSend(tag_queue, "File: ", 0);
    xQueueSend(tag_queue, "Band: ", 0);
    xQueueSend(tag_queue, "Artist: ", 0);
    xQueueSend(tag_queue, "Album: ", 0);
    xQueueSend(tag_queue, "Title: ", 0);

    char tmp[QUEUE_MSG_SIZE];
    char filename[PATHNAME_MAX_LEN] = "";
    char dirname[PATHNAME_MAX_LEN] = "";
    char filepath[PATHNAME_MAX_LEN] = "";

    int retry = fc->filecnt;
    while (retry)
    {
        num = clamp1(num, fc->filecnt);
        DEBUG("Trying to play %d...\n", num);

        if (!fc->find_file(num))
        {
            snprintf(tmp, sizeof(tmp)-1, "File: File %d not found", num);
            xQueueSend(tag_queue, tmp, 0);
            DEBUG("no file %d\n", num);
            return fc->curfile;
        }


        if (!fc->file_is_dir(num))
        {
            fc->file_name(num, filename, sizeof(filename));
            //DEBUG("%s\n", filename);

            int x = fc->file_dirname(num, dirname, sizeof(dirname));
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

        retry--;
        if (updown == FAIL_RANDOM)
            num = file_random();
        else
            num += updown;
    }

    if (!retry)
    {
        snprintf(tmp, sizeof(tmp)-1, "File: retry count 0");
        xQueueSend(tag_queue, tmp, 0);
        DEBUG("retry count\n");
        return 0;
    }
    
    playstack_push(num);

    snprintf(tmp, sizeof(tmp)-1, "Index: %2i:%4i/%4i ", cur_fav_num, num, fc->filecnt);
    xQueueSend(tag_queue, tmp, 0);
    
    snprintf(tmp, sizeof(tmp)-1, "Path: %s", dirname);
    xQueueSend(tag_queue, tmp, 0);

    snprintf(tmp, sizeof(tmp)-1, "File: %s", filename);
    xQueueSend(tag_queue, tmp, 0);
    
    return fc->curfile;
}



