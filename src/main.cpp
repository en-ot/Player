// builtin libs
#include <Arduino.h>

#include "debug.h"

// project
#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "controls.h"
#include "strcache.h"


//###############################################################
// gui input and output
//###############################################################

Gui * gui;

playlist * fc;   //playing file
playlist * pl;   //files and dirs playlist


//###############################################################
// globals
//###############################################################

//prefs
int cur_fav_num;

int8_t volume;   
bool shuffle;
bool repeat;
uint32_t filepos;
int next_file;

int next_updown = FAIL_NEXT;

// seek
int file_seek_by;
int8_t volume_old = -2; // -2 guarantees that setVolume is called at the beginning
int next_dir;

//???
bool read_error = false;

//flags
bool need_play_next_dir = false;
bool need_play_next_file = false;

bool need_set_file_pos = false;

unsigned long save_time;
bool need_save_current_file = false;
bool need_save_volume = false;
bool need_save_repeat = false;
bool need_save_shuffle = false;
bool need_save_file_pos = false;

int ui_page = PAGE_INFO;

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

    int type = 0;
    if (filenum == fc->curfile) type = 2;
    else if (dir_level)         type = 1;
    gui->files_highlight(pvGui, pvElem, type);

    return true;
}


//###############################################################
#define DIRS_CACHE_LINES 20
StrCache * dirs_cache;

bool dirs_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int dirnum = nItem+1;

    int index = dirs_cache->get(dirnum);
    int dir_level = 0;
    if (index == CACHE_MISS)
    {
        if (!pl->find_dir(dirnum))
            return false;

        int filenum = pl->curfile;

        char buf[XLISTBOX_MAX_STR] = "# # # # # # # # # # # # # # # ";  // >= DIR_DEPTH*2

        int disp = 0;
        dir_level = pl->level + 1;
        disp = dir_level*2-2;

        pl->file_name(pl->curfile, &buf[disp], sizeof(buf)-disp);
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, buf);

        dirs_cache->put(dirnum, buf, dir_level | (filenum << 16));
    }
    else
    {
        uint32_t flags = dirs_cache->lines[index].flags;
        int filenum = flags >> 16;
        dir_level = flags & 0xFFFF;
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, dirs_cache->lines[index].txt);
    }

    int type = 0;
    if (dirnum == fc->curdir)  type = 1;
    gui->dirs_highlight(pvGui, pvElem, type);

    return true;
}


int dirs_file_num(int dirs_sel)
{
    int dirnum = dirs_sel;
    int index = dirs_cache->get(dirnum);
    if (index == CACHE_MISS)
        return 0;
    int file_num = dirs_cache->lines[index].flags >> 16;
    return file_num;
}


//###############################################################
bool fav_switch(int fav_num, bool init)
{
    DEBUG("switch to fav %d, %d\n", fav_num, init);

    if (!init)
    {
        if (sound_is_playing())
            filepos = sound_current_time();

        sound_stop();

        prefs_save_now(need_save_current_file);
    }

    fav_num = clamp1(fav_num, FAV_MAX);
    cur_fav_num = fav_num;

    prefs_save_curfav(fav_num);
    
    char fav_path[PATHNAME_MAX_LEN] = {0};
    prefs_load_data(fav_num, fav_path, sizeof(fav_path));
    DEBUG("fav path: %s\n", fav_path);
    
    fc->set_root(fav_path);
    pl->set_root(fav_path);
    gui->files_box(pl->filecnt, files_get_item);
    gui->dirs_box(pl->dircnt, dirs_get_item);

    DEBUG("dircnt: %d\n", pl->dircnt);

    gui->fav_select(fav_num);
    gui->fav(fav_num);
    gui->shuffle(shuffle);
    gui->repeat(repeat);
    gui->volume(volume);

    gui->alive(false);
    gui->gain(false);
    gui->index("");
    gui->redraw();

    playstack_init();
    files_cache = new StrCache(FILES_CACHE_LINES);
    dirs_cache = new StrCache(DIRS_CACHE_LINES);

    start_file(next_file, FAIL_NEXT);

    if (filepos)
        need_set_file_pos = true;

    return true;
}


char fav_str[FAV_MAX][XLISTBOX_MAX_STR] = {0};


void fav_set_str(int fav_num, const char * path)
{
    int nItem = fav_num - 1;
    sprintf(fav_str[nItem], "%d ", fav_num);
    strlcat(fav_str[nItem], path, sizeof(fav_str[nItem]));
}


void fav_init()
{
    char tmp[XLISTBOX_MAX_STR];
    int fav_num;
    for (fav_num = 1; fav_num <= FAV_MAX; fav_num++)
    {
        prefs_get_path(fav_num, tmp, sizeof(tmp));
        if (!tmp[0]) strcpy(tmp, "/");
        fav_set_str(fav_num, tmp);
    }
}


void fav_set_path(int fav_num, const char * path)
{
    fav_set_str(fav_num, path);
    prefs_set_path(fav_num, path);
    gui->redraw();
}


bool fav_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int fav_num = nItem + 1;
    
    int type = (fav_num == cur_fav_num) ? 1 : 0;
    gui->fav_highlight(pvGui, pvElem, type);

    strlcpy(pStrItem, fav_str[nItem], nStrItemLen);
    DEBUG("%s\n", pStrItem);
    return true;
}


//###############################################################
// Init steps
//###############################################################
#define STEPS_TOTAL 10
unsigned long step_t0 = 0;

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
    unsigned long t = millis();
    gui->step_progress(curstep, STEPS_TOTAL);
    gui->loop();
    DEBUG("Step %d end (%dms)\n", curstep, (int)(t - step_t0));
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

    begin("gui");
    gui = new Gui();
    end(0);

    begin("sound");
    sound_setup();
    end(1);

    begin("queue");
    tag_queue = xQueueCreate(QUEUE_DEPTH, QUEUE_MSG_SIZE);
    end(2);

    begin("controls");
    controls_init();
    end(3);

    begin("prefs");
    if (controls_defaults())
    {
        prefs_erase_all();
    }
    cur_fav_num = prefs_load_curfav();
    end(4);

    begin("sdcard");
    if (!SD.begin()) 
    {
        gui->error("SDcard init error");
        SD.initErrorHalt(); // SdFat-lib helper function
    }
    end(5);

    begin("fav");
    gui->fav_box(FAV_MAX, fav_get_item);
    fav_init();
    end(6);

    begin("filectrl");
    fc = new playlist();
    end(7);
 
    begin("playlist");
    pl = new playlist();
    end(8);

    end(9);

    begin("start");
    fav_switch(cur_fav_num, true);
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

    len += 2;
    *p = &info[len];
//    DEBUG("%s: %s", tst, p); 
    return true;
}


void print_hex(char * data, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        DEBUG("%2X ", data[i]);
    }
}


void display_loop()
{
    unsigned long t = millis();

    static unsigned long t1 = 0;
    static uint32_t old_pos = 0;

    uint32_t pos = sound_is_playing() ? sound_current_time() : old_pos;

    if ((t - t1 > 2000) || (pos != old_pos))
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
//        print_hex(msg, 20);
//        DEBUG("\n");

        char * p;
             if (cmp(msg, "Artist", &p))    gui->artist(p);
        else if (cmp(msg, "Band", &p))      gui->band(p);
        else if (cmp(msg, "Album", &p))     gui->album(p);
        else if (cmp(msg, "Title", &p))     gui->title(p);
        else if (cmp(msg, "File", &p))      gui->file(p);
        else if (cmp(msg, "Path", &p))      gui->path(p, fc->root_path.c_str());
        else if (cmp(msg, "Index", &p))     gui->index(p);
        return;
    }

    static unsigned long t0 = 0;
    if (t - t0 > 100)
    {
        t0 = t;
        gui->alive(sound_is_playing());
        gui->gain(sound_is_gain());
        return;
    }

    // static unsigned long t2 = 0;
    // if (t - t2 > 1000)
    // {
    //     t2 = t;
    //     gui->scroll();
    //     return;
    // }
}



//###############################################################
// error check
//###############################################################
#define ERROR_CHECK_INTERVAL 2000

void check_loop()
{
    static unsigned long last_time = 0;
    unsigned long  t = millis();
    bool tick = false;
    if (t - last_time > ERROR_CHECK_INTERVAL)
    {
        tick = true;
        last_time = t;
    }

    if (tick && (read_error || SD.card()->errorCode()))
    {
        last_time = millis();
        DEBUG(".");   
        if (SD.begin()) 
        {
            // filectrl_rewind();
            DEBUG("\n");                 
            read_error = false;   
//            need_save_current_file = false;
            next_file = 1;
            need_play_next_file = true;
        }
    }
}


void memory_loop()
{
    static unsigned long t0 = 0;
    static bool need_print = true;
    unsigned long t = millis();

    if (t - t0 < 1000)
        return;

    t0 = t;
    uint32_t freeheap = ESP.getFreeHeap();
    if (freeheap < 50000)
    {
       need_print = true;
    }
    
    if (need_print)
    {
        need_print = false;
        DEBUG("Memory free: %i\n", freeheap);
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
    check_loop();
    memory_loop();
    vTaskDelay(1);      //avoid watchdog
}


