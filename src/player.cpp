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
class PlayerPrivate
{
public:
    PageInfo * info;
    PageDirs * dirs;
    PageFiles * files;
    PageFav * fav;
    PageSys * sys;

    Gui * gui = nullptr;
//    Page * pages[PAGE_MAX] = {0};
};


Player::Player()
{
    p = new PlayerPrivate;
}


void Player::loop()
{
    p->sys->loop2();
    p->info->loop2();

    auto page = *page_ptr(ui_page);
    page->gui_loop();
}


Page ** Player::page_ptr(int page_num)
{
    switch (page_num)
    {
    case PAGE_INFO:     return (Page**)&p->info;
    case PAGE_SYS:      return (Page**)&p->sys;
    case PAGE_DIRS:     return (Page**)&p->dirs;
    case PAGE_FILES:    return (Page**)&p->files;
    case PAGE_FAV:      return (Page**)&p->fav;
    }
    DEBUG("INVALID PAGE %d\n", page_num);
    return nullptr;
}


void Player::set_page(int page_num, Page * page)
{
    *page_ptr(page_num) = page;
}


void Player::set_gui(Gui * gui)
{
    p->gui = gui;
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

    p->files->box(pl->filecnt);
    p->dirs->box(pl->dircnt);

    DEBUG("dircnt: %d\n", pl->dircnt);

    p->info->update();
    p->info->alive(false);
    p->info->gain(false);
    p->info->index("");
    
    playstack_init();
    start_file(next_file, FAIL_NEXT);

    p->fav->goto_cur();
    p->dirs->goto_cur();
    p->files->goto_cur();

    if (filepos)
        need_set_file_pos = true;

    return true;
}


void Player::update()
{
    auto page = *page_ptr(ui_page);
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


void Player::fav_set(const char * path)
{
    int fav_num = p->fav->sel();
    p->fav->set_path(fav_num, path);
}


// int Player::fav_sel()
// {
//     PageFav * page_fav = (PageFav *)*page_ptr(PAGE_FAV);
//     return page_fav->sel();
// }


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
    p->info->shuffle(shuffle);
    prefs_save_delayed(need_save_shuffle);
}


void Player::toggle_repeat()
{
    repeat = !repeat;
    p->info->repeat(repeat);
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
    p->info->volume(volume);
    p->info->gain(true);
    prefs_save_delayed(need_save_volume);
    return true;
}


//###############################################################
uint8_t page_order[] = {PAGE_INFO, PAGE_FAV, PAGE_FILES, PAGE_DIRS};


void Player::page_change(int page_num)
{
    if (ui_page == page_num)
        return;

    ui_page = page_num;
    gui->set_page(ui_page);

    p->info->scroll_reset();

    auto page = *page_ptr(ui_page);
    page->activate();
}


void Player::page_next()
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

    page_change(page_order[i]);
}

