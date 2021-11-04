#include <Arduino.h>
#include "debug.h"

#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "strcache.h"

#include "player.h"


Player player;

Player::Player()
{

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
        play_file_num(n, FAIL_RANDOM);
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
    next_updown = FAIL_NEXT;
    need_play_next_dir = true;
    return true;
}


bool play_dir_prev()
{
    next_dir = fc->curdir - 1;
    next_updown = FAIL_NEXT;
    need_play_next_dir = true;
    return true;
}


// bool play_root_next()
// {
//     fav_switch(prev_fav_num, false);
//     return true;
// }


bool play_root_prev()
{
    fav_switch(prev_fav_num, false);
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


// bool file_seek(int by)
// {
//     const int seek_delay = 10;
//     static int speed = 5;
//     static uint32_t t_seek1 = 0;
//     uint32_t t = millis();

//     if (!by)
//     {
//         if ((int32_t)(t - t_seek1) > seek_delay)
//         {
//             t_seek1 = t;
//             if (speed > 5) 
//                 speed--;
//         }
//         return false;
//     }

//     // if (!sound_is_playing())
//     //     return false;

//     file_seek_by += by * speed;
//     speed += 5;
//     return true;
// }


// bool change_pause()
// {
//     if (sound_is_playing())
//     {
//         sound_pause();
//         filepos = sound_current_time();
//         prefs_save_now(need_save_current_file);
//         return true;
//     }

//     sound_resume();
//     return true;
// }


// bool change_volume(int change)
// {
//     if (!change)
//         return false;

//     int new_volume = volume + change;
//     if (new_volume < 0)     new_volume = 0;
//     if (new_volume > 21)    new_volume = 21;
    
//     if (new_volume == volume)
//         return false;

//     volume = new_volume;
//     gui->volume(volume);
//     gui->gain(true);
//     prefs_save_delayed(need_save_volume);
//     return true;
// }


//###############################################################
bool files_seek(int by)
{
    if (!by)
        return false;
    gui->files_seek(by);
    return true;
}


bool files_pgupdn(int by)
{
    if (!by)
        return false;
    gui->files_seek(by * LISTBOX_LINES);
    return true;
}


bool files_dir_prev()
{
    pl->find_file(gui->files_sel);
    pl->find_dir(pl->curdir-1);
    //DEBUG("goto %d\n", pl->curfile);
    gui->files_select(pl->curfile);
    return true;
}


bool files_dir_next()
{
    pl->find_file(gui->files_sel);
    pl->find_dir(pl->curdir+1);
    //DEBUG("goto %d\n", pl->curfile);
    gui->files_select(pl->curfile);
    return true;
}


bool files_goto_curfile()
{
    gui->files_select(fc->curfile);
    return true;
}


bool files_play_sel()
{
    ui_page = PAGE_INFO;
    gui->page(ui_page);
    play_file_num(gui->files_sel, FAIL_NEXT);
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


//###############################################################
bool fav_seek(int by)
{
    if (!by)
        return false;
    gui->fav_seek(by);
    return true;
}


bool fav_pgupdn(int by)
{
    if (!by)
        return false;
    gui->fav_seek(by * LISTBOX_LINES);
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


bool fav_goto_curfav()
{
    gui->fav_select(cur_fav_num);
    return true;
}


//###############################################################
bool dirs_pgupdn(int by)
{
    if (!by)
        return false;
    gui->dirs_seek(by * LISTBOX_LINES);
    return true;
}


bool dirs_seek(int by)
{
    if (!by)
        return false;
    gui->dirs_seek(by);
    return true;
}


bool dirs_goto_curdir()
{
    gui->dirs_select(fc->curdir);
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


//###############################################################
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

    //sound_pause();

    gui->page(ui_page);
    return true;
}


//###############################################################
bool sys_page()
{
    //sound_pause();
    ui_page = PAGE_SYS;
    gui->page(ui_page);
    return true;
}


bool sys_seek(int by)
{
    if (!by)
        return false;
    gui->sys_seek(by);
    return true;
}


bool sys_volupdn(int by)
{
    if (!by)
        return false;
    if (by > 0)
        sys_control(gui->sys_sel, 1);
    else
        sys_control(gui->sys_sel, 2);
    return true;
}


bool sys_volshort()
{
    sys_control(gui->sys_sel, 3);
    return true;
}


bool cal_vol()
{
    sys_control(gui->sys_sel, 4);
    return true;
}


bool cal_seek()
{
    sys_control(gui->sys_sel, 5);
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

// Controls info_ctrl = {{
//     change_volume,     file_seek   },{      // volume,      seek
//     play_file_prev,    play_file_next,      // vol_long,    vol_short
//     play_root_next,    change_pause,        // seek_long,   seek_short
//     play_file_down,    change_page,         // b1_long,     b1_short
//     toggle_shuffle,    play_dir_prev,       // b2_long,     b2_short
//     toggle_repeat,     play_dir_next,       // b3_long,     b3_short
// }};

Controls info_ctrl = {{
    0,     0   },{      // volume,      seek
    0,    0,      // vol_long,    vol_short
    0,    0,        // seek_long,   seek_short
    0,    0,         // b1_long,     b1_short
    0,    0,       // b2_long,     b2_short
    0,     0,       // b3_long,     b3_short
}};


Controls files_ctrl = {{
    files_pgupdn,       files_seek   },{    // volume,      seek
    nothing,            files_goto_curfile, // vol_long,    vol_short
    files_set_fav,      files_play_sel,     // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short
    nothing,            files_dir_prev,     // b2_long,     b2_short
    nothing,            files_dir_next,     // b3_long,     b3_short
}};

Controls dirs_ctrl = {{
    dirs_pgupdn,        dirs_seek   },{     // volume,      seek      
    nothing,            dirs_goto_curdir,   // vol_long,    vol_short 
    dirs_set_fav,       dirs_play_sel,        // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short  
    nothing,            nothing,            // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short  
}};

Controls fav_ctrl = {{
    fav_pgupdn,         fav_seek   },{      // volume,      seek      
    nothing,            fav_goto_curfav,    // vol_long,    vol_short 
    fav_reset,          fav_set_num,        // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short  
    nothing,            sys_page,           // b2_long,     b2_short
    nothing,            nothing,            // b3_long,     b3_short  
}};

Controls sys_ctrl = {{
    sys_volupdn,        sys_seek   },{      // volume,      seek      
    nothing,            sys_volshort,       // vol_long,    vol_short 
    nothing,            nothing,            // seek_long,   seek_short
    nothing,            change_page,        // b1_long,     b1_short  
    nothing,            cal_vol,            // b2_long,     b2_short
    nothing,            cal_seek,           // b3_long,     b3_short  
}};


Controls controls[PAGE_MAX] = 
{
    info_ctrl,
    files_ctrl,
    fav_ctrl,
    dirs_ctrl,
    sys_ctrl,
};


class CtrlPage
{
public:
    virtual bool vol(int change)    {return false;}
    virtual bool vol_short()        {return false;}
    virtual bool vol_long()         {return false;}

    virtual bool seek(int change)   {return false;}
    virtual bool seek_short()       {return false;}
    virtual bool seek_long()        {return false;}

    virtual bool b1_short()         {return false;}
    virtual bool b1_long()          {return false;}

    virtual bool b2_short()         {return false;}
    virtual bool b2_long()          {return false;}

    virtual bool b3_short()         {return false;}
    virtual bool b3_long()          {return false;}
};


class CtrlPageInfo : public CtrlPage
{
public:
    bool vol(int change)
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
        gui->gain(true);
        prefs_save_delayed(need_save_volume);
        return true;
    }

    bool vol_short() {return play_file_next();}

    bool vol_long() //play_file_prev
    {
        DEBUG("vol_long\n");
        if (shuffle)
        {
            int n = playstack_pop();
            if (n == 0)
                n = fc->curfile;
            play_file_num(n, FAIL_RANDOM);
        }
        else
        {
            play_file_up();
        }
        return true;
    }

    bool seek_short()
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

    bool seek_long() {fav_switch(prev_fav_num, false); return true;};

    bool seek(int by)
    {
        const int seek_delay = 10;
        static int speed = 5;
        static uint32_t t_seek1 = 0;
        uint32_t t = millis();

        if (!by)
        {
            if ((int32_t)(t - t_seek1) > seek_delay)
            {
                t_seek1 = t;
                if (speed > 5) 
                    speed--;
            }
            return false;
        }

        file_seek_by += by * speed;
        speed += 5;
        return true;
    }

    bool b1_long() {return play_file_down();}
    bool b1_short() {return change_page();}

    bool b2_long() {return toggle_shuffle();}
    bool b2_short() {return play_dir_prev();}

    bool b3_long() {return toggle_repeat();}
    bool b3_short() {return play_dir_next();}

} page_info;


CtrlPage * ctrl_pages[] = {
    &page_info,
};



bool Player::input(InputType type, int key)
{
    if (ui_page == PAGE_INFO)
    {
        CtrlPage * page = ctrl_pages[ui_page];
        if (type == I_BUTTON) 
        {
            DEBUG("BTN\n");
            switch (key)
            {
            case KEY_SEEKSHORT: return page->seek_short();
            case KEY_SEEKLONG:  return page->seek_long();
            case KEY_VOLSHORT:  return page->vol_short();
            case KEY_VOLLONG:   return page->vol_long();
            case KEY_B1LONG:    return page->b1_long();
            case KEY_B1SHORT:   return page->b1_short();
            case KEY_B2LONG:    return page->b2_long();
            case KEY_B2SHORT:   return page->b2_short();
            case KEY_B3LONG:    return page->b3_long();
            case KEY_B3SHORT:   return page->b3_short();
            }

            return false;
        }
        if (type == I_SEEK1)    return page->vol(key);
        if (type == I_SEEK2)    return page->seek(key);
        return false;
    }

    Controls * ctrl = &controls[ui_page];
    if (type == I_BUTTON) return ctrl->keys[key-E_TOTAL]();
    if (type == I_SEEK1)  return ctrl->encoders[E_VOLUME](key);
    if (type == I_SEEK2)  return ctrl->encoders[E_SEEK](key);
    return false;
}


bool input(InputType type, int key)
{
    return player.input(type, key);
}


/*
    //next_file = fc->curfile + 1;

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
        input(I_BUTTON, KEY_VOLSHORT);
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

    if (r == 'm')
    {
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        heap_caps_print_heap_info(MALLOC_CAP_EXEC);
        heap_caps_print_heap_info(MALLOC_CAP_32BIT);
    }


    if (r == 'f')
    {
        sound_stop();
        prefs_erase_all();
    }

    if (r == 'c')
    {
        controls_calibrate(1);
    }

    if (r == 'v')
    {
        controls_calibrate(2);
    }
    
*/
