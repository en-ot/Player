#include <Arduino.h>

#include "InputButton.h"
#include "AnalogEncoder.h"

#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "strcache.h"

InputButton btn_1(BTN_1, true, ACTIVE_LOW);
InputButton btn_2(BTN_2, true, ACTIVE_LOW);
InputButton btn_3(BTN_3, true, ACTIVE_LOW);

AnalogEncoder enc1(AENC1);
AnalogEncoder enc2(AENC2);


//###############################################################
// Input controls
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


void controls_init()
{
    xTaskCreatePinnedToCore(enc_task, "enc_task", 5000, NULL, 2, &enc_task_handle, 0);
}


bool controls_defaults()
{
    return btn_1.isPressed();
}


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
    play_file_num(fc->curfile - 1, FAIL_PREV);
    return true;
}


bool play_file_down()
{
    play_file_num(fc->curfile + 1, FAIL_NEXT);
    return true;
}


int file_random()
{
    return random(1, fc->filecnt+1);
}


bool play_file_random()
{
    int n = file_random();
    while (n == fc->curfile && fc->filecnt > 1)
    {
        n = file_random();
    }

    play_file_num(n, FAIL_RANDOM);
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
        play_file_num(n, FAIL_NEXT);
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
    next_updown = FAIL_PREV;
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


bool files_seek(int by)
{
    if (!by)
        return false;
    gui->files_seek(by);
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


uint8_t page_order[] = {PAGE_INFO, PAGE_FAV, PAGE_FILES, PAGE_DIRS};

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


bool files_play_sel()
{
    ui_page = PAGE_INFO;
    gui->page(ui_page);
    play_file_num(gui->files_sel, FAIL_NEXT);
    return true;
}


bool files_goto_curfile()
{
    gui->files_select(fc->curfile);
    return true;
}


bool dirs_goto_curdir()
{
    gui->dirs_select(fc->curdir);
    return true;
}


bool files_set_fav()
{
    int file_num = gui->files_sel;
    if (!pl->file_is_dir(file_num))
        return false;

    char path[PATHNAME_MAX_LEN];
    pl->file_dirname(file_num, path, sizeof(path));

    int fav_num = gui->fav_sel;
    fav_set_path(fav_num, path);

    ui_page = PAGE_FAV;
    gui->page(ui_page);

    return true;
}


bool dirs_set_fav()
{
    int file_num = dirs_file_num(gui->dirs_sel);
    if (!file_num)
        return false;

    char path[PATHNAME_MAX_LEN];
    pl->file_dirname(file_num, path, sizeof(path));

    int fav_num = gui->fav_sel;
    fav_set_path(fav_num, path);

    ui_page = PAGE_FAV;
    gui->page(ui_page);

    return true;
}


bool dirs_play_sel()
{
    int file_num = dirs_file_num(gui->dirs_sel);
    if (!file_num)
        return false;

    ui_page = PAGE_INFO;
    gui->page(ui_page);
    play_file_num(file_num, FAIL_NEXT);
    return true;
}


bool fav_reset()
{
    int fav_num = gui->fav_sel;
    fav_set_path(fav_num, "/");
    return true;
}


bool fav_set_num()
{
    int fav_num = gui->fav_sel;
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

Controls files_ctrl = {{
    change_volume,      files_seek   },{     // volume,      seek
    nothing,            files_goto_curfile,  // vol_long,    vol_short
    files_set_fav,      files_play_sel,       // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short
    nothing,            nothing,            // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short
}};

Controls fav_ctrl = {{
    change_volume,      fav_seek   },{      // volume,      seek      
    nothing,            nothing,            // vol_long,    vol_short 
    fav_reset,          fav_set_num,        // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short  
    nothing,            nothing,            // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short  
}};

Controls dirs_ctrl = {{
    change_volume,      dirs_seek   },{     // volume,      seek      
    nothing,            dirs_goto_curdir,   // vol_long,    vol_short 
    dirs_set_fav,       dirs_play_sel,        // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short  
    nothing,            nothing,            // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short  
}};

Controls controls[PAGE_MAX] = 
{
    info_ctrl,
    files_ctrl,
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
        //     DEBUG("Error Reset!\n");
        // }
    }

    if (r == 'f')
    {
        sound_stop();
        prefs_erase_all();
    }
}


void controls_loop()
{
    input_loop();
    serial_loop();
}