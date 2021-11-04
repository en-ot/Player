// builtin libs
#include <Arduino.h>

#include "SD_Libs.h"

#include "debug.h"

// project
#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "controls.h"
#include "strcache.h"
#include "network.h"
#include "firmware.h"

//#include "page_info.h"
//#include "page_files.h"
#include "page_dirs.h"
#include "page_fav.h"
#include "page_sys.h"

#include "player.h"

//###############################################################
// gui input and output
//###############################################################

Gui * gui;

Playlist * fc;   //playlist for file playing
Playlist * pl;   //playlist for display


//###############################################################
// globals
//###############################################################

Player * player;

//flags
bool need_play_next_dir = false;
bool need_play_next_file = false;

bool need_set_file_pos = false;

uint32_t save_time;
bool need_save_current_file = false;
bool need_save_volume = false;
bool need_save_repeat = false;
bool need_save_shuffle = false;
bool need_save_file_pos = false;

#define QUEUE_DEPTH 20
QueueHandle_t tag_queue;




//###############################################################
#define FILES_CACHE_LINES 20
StrCache * files_cache;

bool files_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int filenum = nItem+1;

    int index = files_cache->get(filenum);
    int dir_level = 0;
    if (index == CACHE_MISS)
    {
        if (!pl->find_file(filenum))
            return false;

        char buf[XLISTBOX_MAX_STR] = "# # # # # # # # # # # # # # # ";  // >= DIR_DEPTH*2

        int disp = 0;
        if (pl->file_is_dir(filenum))
        {
            dir_level = pl->level + 1;
            disp = dir_level*2-2;
        }

        pl->file_name(filenum, &buf[disp], sizeof(buf)-disp);
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, buf);

        files_cache->put(filenum, buf, dir_level);
    }
    else
    {
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, files_cache->lines[index].txt);
        dir_level = files_cache->lines[index].flags;
    }
    if (nItem == 0)
    {
        int p = strlen(pStrItem);
        snprintf(&pStrItem[p], nStrItemLen-p, " [%d]", pl->filecnt);
    }

    int type = 0;
    if (filenum == fc->curfile) type = 2;
    else if (dir_level)         type = 1;
    gui->files_highlight(pvGui, pvElem, type);

    return true;
}


//###############################################################
bool fav_switch(int fav_num, bool init)
{
    DEBUG("switch to fav %d, %d\n", fav_num, init);

    if (!init)
    {
        if (sound_is_playing())
            player->filepos = sound_current_time();

        sound_stop();

        prefs_save_now(need_save_current_file);

        player->prev_fav_num = player->cur_fav_num;
        prefs_save_main(fav_num, player->prev_fav_num, sys.sd_free_mb);
    }

    fav_num = clamp1(fav_num, FAV_MAX);
    player->cur_fav_num = fav_num;

    char fav_path[PATHNAME_MAX_LEN] = {0};
    prefs_load_data(fav_num, fav_path, sizeof(fav_path));
    DEBUG("fav path: %s\n", fav_path);
    
    fc->set_root(fav_path);
    pl->set_root(fav_path);
    gui->files_box(pl->filecnt, files_get_item);
    page_dirs.box(pl->dircnt);

    DEBUG("dircnt: %d\n", pl->dircnt);

    gui->fav(fav_num);
    gui->shuffle(player->shuffle);
    gui->repeat(player->repeat);
    gui->volume(player->volume);
    gui->alive(false);
    gui->gain(false);
    gui->index("");
    gui->redraw();

    playstack_init();
    if (!files_cache)
        files_cache = new StrCache(FILES_CACHE_LINES);
    else
        files_cache->clear();

    start_file(player->next_file, FAIL_NEXT);

    page_fav.goto_cur();
    page_dirs.goto_cur();
    player->files_goto_curfile();

    if (player->filepos)
        need_set_file_pos = true;

    return true;
}


//###############################################################
// Init steps
//###############################################################
#define STEPS_TOTAL 10
uint32_t step_t0 = 0;

void begin(const char * step_name)
{
    DEBUG("Init step: %s\n", step_name);
    if (gui)
    {
        gui->step_begin(step_name);
    }
    step_t0 = millis();
};


void end(int curstep)
{
    int32_t t = millis();
    gui->step_progress(curstep, STEPS_TOTAL);
    gui->loop();
    DEBUG("Step %d end (%dms)\n", curstep, (int)(t - step_t0));
}


bool player_input(PlayerInputType type, int key)
{
    return player->input(type, key);
}


//###############################################################
// Setup
//###############################################################
void setup() 
{
    Serial.begin(115200);
    while (!Serial) 
    {       // Wait for USB Serial
        SysCall::yield();
    }
    DEBUG("\n");

    // uint8_t *p1 = (uint8_t*)malloc(50000);
    // Serial.printf("%08X\n", (int)p1);
    // uint8_t *p2 = (uint8_t*)malloc(30000);
    // Serial.printf("%08X\n", (int)p2);

    player = new Player();

    begin("gui");
    gui = new Gui();
    player->set_gui(gui);
    end(0);

    // free(p1);
    // free(p2);

    begin("sound");
    sound_setup();
    end(1);

    begin("queue");
    tag_queue = xQueueCreate(QUEUE_DEPTH, QUEUE_MSG_SIZE);
    end(2);

    begin("controls");
    controls_init(&player_input);
    end(3);

    begin("prefs");
    if (controls_defaults())
    {
        prefs_erase_all();
    }
    prefs_load_main(&player->cur_fav_num, &player->prev_fav_num, &sys.sd_free_mb);
    //prefs_open_fav(cur_fav_num);
    //Serial.printf("cur:%d prev:%d\n", cur_fav_num, prev_fav_num);
    end(4);

    begin("sdcard");
    if (!SD.begin()) 
    {
        gui->error("SDcard init error");
        SD.initErrorHalt(); // SdFat-lib helper function
    }
    if (!sys.sd_free_mb)
    {
        calc_sd_free_size();
        prefs_save_main(player->cur_fav_num, player->prev_fav_num, sys.sd_free_mb);
    }

    end(5);

    begin("fav");
    page_fav.box();
    end(6);

    begin("filectrl");
    fc = new Playlist(DIRECT_ACCESS);
    end(7);
 
    begin("Playlist");
    pl = new Playlist(SAFE_ACCESS);
    end(8);

    begin("network");
    network_init();
    end(9);

    begin("start");
    fav_switch(player->cur_fav_num, true);
    end(10);
}


//###############################################################
// Display : Main Task
//###############################################################
bool cmp(char * info, const char * tst, char ** p)
{
    int len = strlen(tst);
    if (strncmp(info, tst, len))
        return false;

    *p = &info[len];
//    DEBUG("%s: %s", tst, p); 
    return true;
}


void display_loop()
{
    uint32_t t = millis();

    static uint32_t t1 = 0;
    static uint32_t old_pos = 0;

//    uint32_t pos = sound_is_playing() ? sound_current_time() : old_pos;
    uint32_t pos = sound_current_time();

    if (((int32_t)(t - t1) > 2000) || (pos != old_pos))
    {
        t1 = t;
        old_pos = pos;
        gui->time_progress(pos, sound_duration());
        return;
    }

    char msg[QUEUE_MSG_SIZE];
    BaseType_t res = xQueueReceive(tag_queue, &msg, 0);
    msg[QUEUE_MSG_SIZE-1] = 0;
    if (res == pdTRUE)
    {
        DEBUG("id3data[%i]: %s\n", strlen(msg), msg);
        //DEBUG_DUMP8(msg, len, len);
//        DEBUG("\n");

        char * p;
             if (cmp(msg, "Artist: ", &p))    gui->artist(p);
        else if (cmp(msg, "Band: ", &p))      gui->band(p);
        else if (cmp(msg, "Album: ", &p))     gui->album(p);
        else if (cmp(msg, "Title: ", &p))     gui->title(p);
        else if (cmp(msg, "File: ", &p))      gui->file(p);
        else if (cmp(msg, "Path: ", &p))      gui->path(p, fc->root_path.c_str());
        else if (cmp(msg, "Index: ", &p))     gui->index(p);
        return;
    }

    static uint32_t t0 = 0;
    if ((int32_t)(t - t0) > 100)
    {
        t0 = t;
        gui->alive(sound_is_playing());
        gui->gain(sound_is_gain());
        return;
    }
}


//###############################################################
// error check
//###############################################################
#define ERROR_CHECK_INTERVAL 2000

void check_loop()
{
    static uint32_t last_time = 0;
    uint32_t t = millis();
    bool tick = false;
    if ((int32_t)(t - last_time) > ERROR_CHECK_INTERVAL)
    {
        tick = true;
        last_time = t;
    }

    if (tick && (sys.read_error || SD.card()->errorCode()))
    {
        last_time = millis();
        DEBUG(".");   
        if (SD.begin()) 
        {
            // filectrl_rewind();
            DEBUG("\n");                 
            sys.read_error = false;   
//            need_save_current_file = false;
            player->next_file = 1;
            need_play_next_file = true;
        }
    }
}


//###############################################################
// Main Loop
//###############################################################
void loop()
{
    controls_loop();
    display_loop();
    gui->loop();
    prefs_loop();
    network_loop();
    check_loop();
    page_sys.loop();
    yield();
    //vTaskDelay(1);      //avoid watchdog
}


void main_pause()
{
    sound_pause();
//    controls_pause();
}


void main_restart()
{
    fav_switch(player->cur_fav_num, false);
}


void main_resume()
{
    sound_resume();
//    controls_resume();
}

