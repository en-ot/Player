#include <Arduino.h>
#include "debug.h"

#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "strcache.h"

#include "page_fav.h"

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


extern CtrlPage * ctrl_pages[PAGE_MAX];

void Player::set_ctrl(int page, CtrlPage * ctrl)
{
    //DEBUG("%d:%X\n", page, ctrl);
    ctrl_pages[page] = ctrl;
    //DEBUG_DUMP32(ctrl_pages, 5, 5);
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


void Player::reset_to_defaults()
{
    sound_stop();
    prefs_erase_all();
}


void Player::toggle_pause()
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
uint8_t page_order[] = {PAGE_INFO, PAGE_FAV, PAGE_FILES, PAGE_DIRS};


void Player::set_page(int page)
{
    ui_page = page;
    gui->set_page(ui_page);
}


void Player::next_page()
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

    set_page(page_order[i]);
}

