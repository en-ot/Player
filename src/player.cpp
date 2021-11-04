#include <Arduino.h>
#include "debug.h"

#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "strcache.h"

#include "player.h"


//###############################################################
int file_random()
{
    return random(1, fc->filecnt+1);
}


//###############################################################
Player::Player()
{
    player_ctrl_init();
}


void Player::set_gui(Gui * gui)
{
    _gui = gui;
}


bool Player::input(PlayerInputType type, int key)
{
    if (ui_page >= PAGE_MAX)
        return false;

    return player_ctrl_input(ui_page, type, key);
}


void Player::play_file_num(int num, int updown)
{
    num = clamp1(num, fc->filecnt);
    next_file = num;
    next_updown = updown;
    need_play_next_file = true;
}


void Player::play_file_up()
{
    play_file_num(fc->curfile - 1, FAIL_PREV);
}


void Player::play_file_down()
{
    play_file_num(fc->curfile + 1, FAIL_NEXT);
}


void Player::play_file_random()
{
    int n = file_random();
    while (n == fc->curfile && fc->filecnt > 1)
    {
        n = file_random();
    }

    play_file_num(n, FAIL_RANDOM);
}


void Player::play_file_next()
{
    if (shuffle)
        play_file_random();
    else
        play_file_down();
}


void Player::play_file_prev()
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
}


void Player::play_dir_next()
{
    next_dir = fc->curdir + 1;
    next_updown = FAIL_NEXT;
    need_play_next_dir = true;
}


void Player::play_dir_prev()
{
    next_dir = fc->curdir - 1;
    next_updown = FAIL_NEXT;
    need_play_next_dir = true;
}


void Player::play_root_next()
{
    fav_switch(prev_fav_num, false);
}


void Player::play_root_prev()
{
    fav_switch(prev_fav_num, false);
}


void Player::toggle_shuffle()
{
    shuffle = !shuffle;
    gui->shuffle(shuffle);
    prefs_save_delayed(need_save_shuffle);
}


void Player::toggle_repeat()
{
    repeat = !repeat;
    gui->repeat(repeat);
    prefs_save_delayed(need_save_repeat);
}


bool Player::file_seek(int by)
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

    // if (!sound_is_playing())
    //     return false;

    file_seek_by += by * speed;
    speed += 5;
    return true;
}


void Player::change_pause()
{
    if (!sound_is_playing())
    {
        sound_resume();
        return;
    }
        
    sound_pause();
    filepos = sound_current_time();
    prefs_save_now(need_save_current_file);
}


bool Player::change_volume(int change)
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


//###############################################################
bool Player::files_seek(int by)
{
    if (!by) 
        return false;
    gui->files_seek(by);
    return true;
}


bool Player::files_pgupdn(int by)
{
    if (!by)
        return false;
    gui->files_seek(by * LISTBOX_LINES);
    return true;
}


void Player::files_dir_prev()
{
    pl->find_file(gui->files_sel);
    pl->find_dir(pl->curdir-1);
    //DEBUG("goto %d\n", pl->curfile);
    gui->files_select(pl->curfile);
}


void Player::files_dir_next()
{
    pl->find_file(gui->files_sel);
    pl->find_dir(pl->curdir+1);
    //DEBUG("goto %d\n", pl->curfile);
    gui->files_select(pl->curfile);
}


void Player::files_goto_curfile()
{
    gui->files_select(fc->curfile);
}


void Player::files_play_sel()
{
    ui_page = PAGE_INFO;
    gui->page(ui_page);
    play_file_num(gui->files_sel, FAIL_NEXT);
}


void Player::files_set_fav()
{
    int file_num = gui->files_sel;
    if (!pl->file_is_dir(file_num))
        return;

    char path[PATHNAME_MAX_LEN];
    pl->file_dirname(file_num, path, sizeof(path));

    int fav_num = gui->fav_sel;
    fav_set_path(fav_num, path);

    ui_page = PAGE_FAV;
    gui->page(ui_page);
}


//###############################################################
bool Player::fav_seek(int by)
{
    if (!by)
        return false;
    gui->fav_seek(by);
    return true;
}


bool Player::fav_pgupdn(int by)
{
    if (!by)
        return false;
    gui->fav_seek(by * LISTBOX_LINES);
    return true;
}


void Player::fav_reset()
{
    int fav_num = gui->fav_sel;
    fav_set_path(fav_num, "/");
}


void Player::fav_set_num()
{
    int fav_num = gui->fav_sel;
    fav_switch(fav_num, false);
    ui_page = PAGE_INFO;
    gui->page(ui_page);
}


void Player::fav_goto_curfav()
{
    gui->fav_select(cur_fav_num);
}


//###############################################################
bool Player::dirs_pgupdn(int by)
{
    if (!by)
        return false;
    gui->dirs_seek(by * LISTBOX_LINES);
    return true;
}


bool Player::dirs_seek(int by)
{
    if (!by)
        return false;
    gui->dirs_seek(by);
    return true;
}


void Player::dirs_goto_curdir()
{
    gui->dirs_select(fc->curdir);
}


void Player::dirs_set_fav()
{
    int file_num = dirs_file_num(gui->dirs_sel);
    if (!file_num)
        return;

    char path[PATHNAME_MAX_LEN];
    pl->file_dirname(file_num, path, sizeof(path));

    int fav_num = gui->fav_sel;
    fav_set_path(fav_num, path);

    ui_page = PAGE_FAV;
    gui->page(ui_page);
}


void Player::dirs_play_sel()
{
    int file_num = dirs_file_num(gui->dirs_sel);
    if (!file_num)
        return;

    ui_page = PAGE_INFO;
    gui->page(ui_page);
    play_file_num(file_num, FAIL_NEXT);
}


//###############################################################
uint8_t page_order[] = {PAGE_INFO, PAGE_FAV, PAGE_FILES, PAGE_DIRS};

void Player::change_page()
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
    if (i >= sizeof(page_order)) 
        i = 0;
    ui_page = page_order[i];

    //sound_pause();

    gui->page(ui_page);
}


//###############################################################
void Player::sys_page()
{
    //sound_pause();
    ui_page = PAGE_SYS;
    gui->page(ui_page);
}


bool Player::sys_seek(int by)
{
    if (!by)
        return false;
    gui->sys_seek(by);
    return true;
}


bool Player::sys_volupdn(int by)
{
    if (!by)
        return false;
    if (by > 0)
        sys_control(gui->sys_sel, 1);
    else
        sys_control(gui->sys_sel, 2);
    return true;
}


void Player::sys_volshort()
{
    sys_control(gui->sys_sel, 3);
}


void Player::cal_vol()
{
    sys_control(gui->sys_sel, 4);
}


void Player::cal_seek()
{
    sys_control(gui->sys_sel, 5);
}


