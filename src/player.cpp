#include <Arduino.h>
#include "debug.h"

#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "strcache.h"

#include "page_fav.h"
#include "page_info.h"
#include "page_files.h"
#include "page_dirs.h"
#include "page_sys.h"

#include "player.h"


//###############################################################
int file_random()
{
    return random(1, fc->filecnt+1);
}


//###############################################################
Player::Player()
{
}


void Player::loop()
{
    page_sys.loop2();
    
    PageInfo * page_info = (PageInfo *)*page_ptr(PAGE_INFO);
    page_info->loop2();

    auto page = *page_ptr(ui_page);
    page->gui_loop();
}


Page ** Player::page_ptr(int page_num)
{
    return &pages[page_num];
}


void Player::set_page(int page_num, Page * page)
{
    *page_ptr(page_num) = page;
}


void Player::set_gui(Gui * gui)
{
    _gui = gui;
}


bool Player::input(PlayerInputType type, int key)
{
    if (ui_page >= PAGE_MAX)
        return false;

    auto ctrl = (*page_ptr(ui_page))->ctrl;
    return ctrl->input(type, key);
}


void Player::freeze()
{
    sound_pause();
//    controls_pause();
}


void Player::unfreeze()
{
    sound_resume();
//    controls_resume();
}


//###############################################################
bool Player::fav_switch(int fav_num, bool init)
{
    DEBUG("switch to fav %d, %d\n", fav_num, init);

    if (!init)
    {
        if (sound_is_playing())
            filepos = sound_current_time();

        sound_stop();

        prefs_save_now(need_save_current_file);

        prev_fav_num = cur_fav_num;
        prefs_save_main(fav_num, prev_fav_num, sys.sd_free_mb);
    }

    fav_num = clamp1(fav_num, FAV_MAX);
    cur_fav_num = fav_num;

    char fav_path[PATHNAME_MAX_LEN] = {0};
    prefs_load_data(fav_num, fav_path, sizeof(fav_path));
    DEBUG("fav path: %s\n", fav_path);
    
    fc->set_root(fav_path);
    pl->set_root(fav_path);

    PageFiles * page_files = (PageFiles *)*page_ptr(PAGE_FILES);
    page_files->box(pl->filecnt);

    page_dirs.box(pl->dircnt);

    DEBUG("dircnt: %d\n", pl->dircnt);

    playstack_init();
    start_file(next_file, FAIL_NEXT);

    page_fav.goto_cur();
    page_dirs.goto_cur();
    page_files->goto_cur();

    if (filepos)
        need_set_file_pos = true;

    update();

    return true;
}


void Player::update()
{
    auto page = pages[ui_page];
    page->update();
}


void Player::fav_next()
{
    fav_switch(prev_fav_num, false);
}


void Player::fav_prev()
{
    fav_switch(prev_fav_num, false);
}


void Player::restart()
{
    fav_switch(cur_fav_num, false);
}


//###############################################################
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


void Player::toggle_shuffle()
{
    shuffle = !shuffle;
    PageInfo * page_info = (PageInfo *)*page_ptr(PAGE_INFO);
    page_info->shuffle(shuffle);
    prefs_save_delayed(need_save_shuffle);
}


void Player::toggle_repeat()
{
    repeat = !repeat;
    PageInfo * page_info = (PageInfo *)*page_ptr(PAGE_INFO);
    page_info->repeat(repeat);
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
    PageInfo * page_info = (PageInfo *)*page_ptr(PAGE_INFO);
    page_info->volume(volume);
    page_info->gain(true);
    prefs_save_delayed(need_save_volume);
    return true;
}


//###############################################################
uint8_t page_order[] = {PAGE_INFO, PAGE_FAV, PAGE_FILES, PAGE_DIRS};


void Player::change_page(int page_num)
{
    if (ui_page == page_num)
        return;

    ui_page = page_num;
    gui->set_page(ui_page);

    PageInfo * page_info = (PageInfo *)*page_ptr(PAGE_INFO);
    page_info->scroll_reset();

    auto page = pages[ui_page];
    page->activate();
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

    change_page(page_order[i]);
}

