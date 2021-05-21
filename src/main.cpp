// builtin libs
#include <Arduino.h>

// project-independent libs
#include "InputButton.h"
#include "AnalogEncoder.h"

// project
#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"


//###############################################################
// gui input and output
//###############################################################
InputButton btn_1(BTN_1, true, ACTIVE_LOW);
InputButton btn_2(BTN_2, true, ACTIVE_LOW);
InputButton btn_3(BTN_3, true, ACTIVE_LOW);

AnalogEncoder enc1(AENC1);
AnalogEncoder enc2(AENC2);

Gui * gui;

playlist * fc;   //playing
playlist * pl;   //list


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
int next_updown = +1;

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
// Realtime tasks
//###############################################################
TaskHandle_t enc_task_handle;
static void enc_task(void * pvParameters)
{
    while (true)
    {
        enc1.process();
        enc2.process();
        vTaskDelay(1);
    }
}


TaskHandle_t audio_task_handle;
static void audio_task(void * pvParameters)
{
    while (true)
    {
        sound_task();
        vTaskDelay(1);      //5 ok, 7 bad
    }
}


//###############################################################
// Setup
//###############################################################
void display_header()
{
    gui->alive(true);
    gui->shuffle(shuffle);
    gui->repeat(repeat);
    gui->volume(volume);
    gui->index("");
}


//###############################################################
#define LIST_CACHE_LINES 10

typedef struct
{
    int key;
    int access;
    int flags;
    char txt[XLISTBOX_MAX_STR];
} CacheLine;

CacheLine list_lines[LIST_CACHE_LINES] = {0};

typedef struct
{
    int cnt;
    int access;
    CacheLine * lines;
} ListboxCache;

ListboxCache list_cache = {LIST_CACHE_LINES, 0, list_lines};

#define CACHE_MISS -1
#define CACHE_EMPTY 0

int cache_get_item(ListboxCache * cache, int key)
{
    int i;
    for (i = 0; i < cache->cnt; i++)
    {
        if (cache->lines[i].key == key)
        {
            cache->lines[i].access = ++cache->access;
            return i;
        }
    }
    return CACHE_MISS;
}


void cache_put_item(ListboxCache * cache, int key, char * buf, int flags)
{
    int oldest_index = 0;
    int oldest_time = 0;
    int i;
    for (i = 0; i < cache->cnt; i++)
    {
        if (cache->lines[i].key == CACHE_EMPTY)
        {
            oldest_index = i;
            break;
        }
        int time = cache->access - cache->lines[i].access;
        if (time > oldest_time)
        {
            oldest_time = time;
            oldest_index = i;
        }
    }
    CacheLine * line = &cache->lines[oldest_index];
    line->key = key;
    line->access = ++cache->access;
    line->flags = flags;
    memcpy(line->txt, buf, sizeof(line->txt));
}


void cache_init(ListboxCache * cache)
{
    cache->access = 0;
    memset(cache->lines, 0, sizeof(CacheLine) * cache->cnt);
}

//###############################################################
bool list_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int filenum = nItem+1;

    int index = cache_get_item(&list_cache, filenum);
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

        cache_put_item(&list_cache, filenum, buf, dir_level);

        //Serial.println(buf);
    }
    else
    {
        snprintf(pStrItem, nStrItemLen, "%d:%s", filenum, list_cache.lines[index].txt);
        dir_level = list_cache.lines[index].flags;
    }

    int type = 0;
    if (filenum == fc->curfile) type = 2;
    else if (dir_level)         type = 1;

    gui->list_highlight(pvGui, pvElem, type);
    return true;
}


//###############################################################
bool dirs_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    snprintf(pStrItem, nStrItemLen, "%d:Dir", nItem);
    return true;
}


//###############################################################
bool fav_switch(int fav_num, bool init)
{
    Serial.printf("switch to fav %d, %d\n", fav_num, init);

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
    Serial.printf("fav path: %s\n", fav_path);
    
    fc->set_root(fav_path);
    pl->set_root(fav_path);
    gui->list_box(pl->filecnt, list_get_item);
    gui->fav_set(fav_num);
    gui->redraw();

    display_header();

    playstack_init();
    cache_init(&list_cache);

    start_file(next_file, +1);

    if (filepos)
        need_set_file_pos = true;

    return true;
}


//###############################################################
char fav_str[FAV_MAX][XLISTBOX_MAX_STR] = {0};

void fav_init()
{

}

void fav_set_path(int fav_num, const char * path)
{
    int nItem = fav_num - 1;
    strlcpy(fav_str[nItem], path, sizeof(fav_str[nItem]));
    prefs_set_path(fav_num, path);
    gui->redraw();
}


bool fav_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int fav_num = nItem + 1;
    if (!fav_str[nItem][0])
    {
        int x = sprintf(fav_str[nItem], "%d ", fav_num);

        prefs_get_path(fav_num, &fav_str[nItem][x], nStrItemLen-x);
        if (!fav_str[nItem][x])
        {
            strcpy(&fav_str[nItem][x], "/");
        }
    }

    int type = (fav_num == cur_fav_num) ? 1 : 0;
    gui->fav_highlight(pvGui, pvElem, type);

    strlcpy(pStrItem, fav_str[nItem], nStrItemLen);
    Serial.println(pStrItem);
    return true;
}


//###############################################################
#define STEP_TOTAL 8
void step(int curstep)
{
    static unsigned long t0 = 0;
    unsigned long t = millis();
    gui->step_progress(curstep, STEP_TOTAL);
    gui->loop();
    Serial.printf("Step %d:%dms\n", curstep, (int)(t - t0));
    t0 = t;
}


void setup() 
{
    Serial.begin(115200);
    while (!Serial) 
    {       // Wait for USB Serial
        SysCall::yield();
    }
    Serial.println();

    gui = new Gui();
    step(0);

    sound_setup();
    step(1);

    xTaskCreatePinnedToCore(enc_task, "enc_task", 5000, NULL, 2, &enc_task_handle, 0);
    xTaskCreatePinnedToCore(audio_task, "audio_task", 5000, NULL, 2, &audio_task_handle, 1);
    tag_queue = xQueueCreate(QUEUE_DEPTH, QUEUE_MSG_SIZE);
    step(2);

    if (btn_1.isPressed())
    {
        prefs_erase_all();
    }
    cur_fav_num = prefs_load_curfav();
    step(3);

    if (!SD.begin()) 
    {
        SD.initErrorHalt(); // SdFat-lib helper function
    }
    step(4);

    gui->fav_box(FAV_MAX, fav_get_item);
    gui->dirs_box(15, dirs_get_item);
    step(5);

    fav_init();
    step(6);

    fc = new playlist();
    pl = new playlist();
    step(7);

    fav_switch(cur_fav_num, true);
    step(8);
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
//    Serial.printf("%s: %s", tst, p); 
    return true;
}


void print_hex(char * data, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        Serial.printf("%2X ", data[i]);
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
        Serial.printf("id3data[%i]: %s\n", strlen(msg), msg);
//        print_hex(msg, 20);
//        Serial.println();

        char * p;
             if (cmp(msg, "Artist", &p))    gui->artist(p);
        else if (cmp(msg, "Band", &p))      gui->band(p);
        else if (cmp(msg, "Album", &p))     gui->album(p);
        else if (cmp(msg, "Title", &p))     gui->title(p);
        else if (cmp(msg, "File", &p))      gui->file(p);
        else if (cmp(msg, "Path", &p))      gui->path(p);
        else if (cmp(msg, "Index", &p))     gui->index(p);
        return;
    }

    static unsigned long t0 = 0;
    if (t - t0 > 100)
    {
        t0 = t;
        gui->alive(sound_is_playing());
        return;
    }
}


//###############################################################
// Input controls
//###############################################################
bool play_file_num(int num, int updown)
{
    num = clamp1(num, fc->filecnt);
    next_file = num;
    next_updown = updown;
    need_play_next_file = true;
    return true;
}


bool play_file_up(void)
{
    play_file_num(fc->curfile - 1, -1);
    return true;
}


bool play_file_down()
{
    play_file_num(fc->curfile + 1, +1);
    return true;
}


bool play_file_random()
{
    int n = random(1, fc->filecnt+1);
    while (n == fc->curfile && fc->filecnt > 1)
    {
        n = random(1, fc->filecnt+1);
    }

    play_file_num(n, +1);
    return true;
}


bool play_file_next()
{
    if (shuffle)
        play_file_random();
    else
        play_file_down();
    return true;
}


bool play_file_prev()
{
    if (shuffle)
    {
        int n = playstack_pop();
        if (n == 0)
            n = fc->curfile;
        play_file_num(n, +1);
    }
    else
    {
        play_file_up();
    }
    return true;
}


bool play_dir_next()
{
    next_dir = fc->curdir + 1;
    need_play_next_dir = true;
    return true;
}


bool play_dir_prev()
{
    next_dir = fc->curdir - 1;
    need_play_next_dir = true;
    return true;
}


bool play_root_next()
{
    fav_switch(cur_fav_num + 1, false);
    return true;
}


bool play_root_prev()
{
    fav_switch(cur_fav_num - 1, false);
    return true;
}


bool toggle_shuffle()
{
    shuffle = !shuffle;
    gui->shuffle(shuffle);
    prefs_save_delayed(need_save_shuffle);
    return true;
}


bool toggle_repeat()
{
    repeat = !repeat;
    gui->repeat(repeat);
    prefs_save_delayed(need_save_repeat);
    return true;
}


bool file_seek(int by)
{
    const int seek_delay = 10;
    static int speed = 5;
    static unsigned long t_seek1 = 0;
    unsigned long t = millis();

    if (!by)
    {
        if (t - t_seek1 > seek_delay)
        {
            t_seek1 = t;
            if (speed > 5) 
                speed--;
        }
        return false;
    }

    if (!sound_is_playing())
        return false;

    file_seek_by += by * speed;
    speed += 5;
    return true;
}


bool list_seek(int by)
{
    if (!by)
        return false;
    gui->list_seek(by);
    return true;
}


bool fav_seek(int by)
{
    if (!by)
        return false;
    gui->fav_seek(by);
    return true;
}


bool dirs_seek(int by)
{
    if (!by)
        return false;
    gui->dirs_seek(by);
    return true;
}


bool change_pause()
{
    if (sound_is_playing())
    {
        sound_pause();
        filepos = sound_current_time();
        prefs_save_now(need_save_current_file);
        return true;
    }

    sound_resume();
    return true;
}


bool change_volume(int change)
{
    if (!change)
        return false;

    int new_volume = volume + change;
    if (new_volume < 0)     new_volume = 0;
    if (new_volume > 21)    new_volume = 21;
    
    if (new_volume == volume)
        return false;

    volume = new_volume;
    gui->volume(volume);
    prefs_save_delayed(need_save_volume);
    return true;
}


uint8_t page_order[] = {PAGE_INFO, PAGE_FAV, PAGE_LIST};

bool change_page()
{
    int i;
    for (i = 0; i < sizeof(page_order); i++)
    {
        if (page_order[i] == ui_page)
        {
            i++;
            break;
        }
    }
    if (i >= sizeof(page_order)) i = 0;
    ui_page = page_order[i];

    sound_pause();

    gui->page(ui_page);
    return true;
}


bool play_selfile()
{
    ui_page = PAGE_INFO;
    gui->page(ui_page);
    play_file_num(gui->list_selfile, +1);
    return true;
}


bool list_goto_curfile()
{
    gui->list_select(fc->curfile);
    return true;
}


bool set_fav()
{
    char path[PATHNAME_MAX_LEN];

    int file_num = gui->list_selfile;
    if (!pl->file_is_dir(file_num))
        return false;

    pl->file_dirname(file_num, path, sizeof(path));

    int fav_num = gui->fav_selfile;
    fav_set_path(fav_num, path);

    return true;
}


bool reset_fav()
{
    int fav_num = gui->fav_selfile;
    fav_set_path(fav_num, "/");
    return true;
}



bool set_fav_num()
{
    int fav_num = gui->fav_selfile;
    fav_switch(fav_num, false);
    ui_page = PAGE_INFO;
    gui->page(ui_page);
    return true;
}


bool nothing()
{
    return true;
}


//###############################################################
enum {
    E_VOLUME,           E_SEEK,
    E_TOTAL
};

enum {
    K_VOLLONG,          K_VOLSHORT,
    K_SEEKLONG,         K_SEEKSHORT, 
    K_B1LONG,           K_B1SHORT, 
    K_B2LONG,           K_B2SHORT, 
    K_B3LONG,           K_B3SHORT,
    K_TOTAL
};

typedef struct
{
    bool (*encoders[E_TOTAL])(int);
    bool (*keys[K_TOTAL])();
} Controls;

Controls info_ctrl = {{
    change_volume,     file_seek   },{      // volume,      seek
    play_file_prev,    play_file_next,      // vol_long,    vol_short
    play_root_next,    change_pause,        // seek_long,   seek_short
    play_dir_prev,     change_page,         // b1_long,     b1_short
    play_dir_next,     play_file_down,      // b2_long,     b2_short
    toggle_repeat,     toggle_shuffle,      // b3_long,     b3_short
}};

Controls list_ctrl = {{
    change_volume,      list_seek   },{     // volume,      seek
    nothing,            list_goto_curfile,  // vol_long,    vol_short
    set_fav,            play_selfile,       // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short
    nothing,            nothing,            // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short
}};

Controls fav_ctrl = {{
    change_volume,      fav_seek   },{      // volume,      seek      
    nothing,            nothing,            // vol_long,    vol_short 
    reset_fav,          set_fav_num,        // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short  
    nothing,            nothing,            // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short  
}};

Controls dirs_ctrl = {{
    change_volume,      dirs_seek   },{     // volume,      seek      
    nothing,            list_goto_curfile,  // vol_long,    vol_short 
    nothing,            nothing,            // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short  
    nothing,            nothing,            // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short  
}};

Controls controls[PAGE_MAX] = 
{
    info_ctrl,
    list_ctrl,
    fav_ctrl,
    dirs_ctrl,
};

bool input_loop()
{
    Controls * ctrl = &controls[ui_page];

    if (ctrl->encoders[E_VOLUME](enc1.get_move())) return true;

    if (ctrl->encoders[E_SEEK](-enc2.get_move())) return true;

    if (enc1.long_press())          return ctrl->keys[K_VOLLONG]();
    if (enc1.short_press())         return ctrl->keys[K_VOLSHORT]();
    
    if (enc2.long_press())          return ctrl->keys[K_SEEKLONG]();
    if (enc2.short_press())         return ctrl->keys[K_SEEKSHORT]();

    if (btn_1.longPress())          return ctrl->keys[K_B1LONG]();
    if (btn_1.shortPress())         return ctrl->keys[K_B1SHORT]();

    if (btn_2.longPress())          return ctrl->keys[K_B2LONG]();
    if (btn_2.shortPress())         return ctrl->keys[K_B2SHORT]();

    if (btn_3.longPress())          return ctrl->keys[K_B3LONG]();
    if (btn_3.shortPress())         return ctrl->keys[K_B3SHORT]();

    return false;
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
        Serial.print('.');   
        if (SD.begin()) 
        {
            // filectrl_rewind();
            Serial.println();                 
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
    if (freeheap < 100000)
        need_print = true;
    
    if (need_print)
    {
        need_print = false;
        Serial.printf("Memory free: %i\n", freeheap);
    }
}


//###############################################################
void serial_loop()
{
    if (!Serial.available()) 
        return;

    char r;
    Serial.read(&r, 1);

    next_file = fc->curfile + 1;

    if (r == 'd')
    {
        play_dir_next();
    }

    if (r == 'a')
    {
        play_dir_prev();
    }

    if (r == 'e')
    {
        play_root_next();
    }

    if (r == 'q')
    {
        play_root_prev();
    }

    if (r == 'r') 
    {
        // if (SD.begin())
        // {
        //     filectrl_rewind();
        //     Serial.println("Error Reset!");
        // }
    }

    if (r == 'f')
    {
        sound_stop();
        prefs_erase_all();
    }
}


//###############################################################
// Main Loop
//###############################################################
void loop()
{
    input_loop();
    serial_loop();
    display_loop();
    gui->loop();
    prefs_loop();
    check_loop();
    memory_loop();
    vTaskDelay(1);      //avoid watchdog
}


