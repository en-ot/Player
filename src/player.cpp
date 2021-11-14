#include <Arduino.h>
#include "debug.h"

#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "strcache.h"
#include "playlist.h"

#include "page_fav.h"
#include "page_info.h"
#include "page_files.h"
#include "page_dirs.h"
#include "page_sys.h"

#include "player.h"

typedef enum
{
    NEED_NOT = 0,
    NEED_START = 1,
    NEED_CONT = 2,
} NeedPlay;


//###############################################################
class PlayerPrivate
{
public:
    PageInfo * info;
    PageDirs * dirs;
    PageFiles * files;
    PageFav * fav;
    PageSys * psys;

    Gui * gui = nullptr;
    Sound * sound = nullptr;

    Playlist * list[2];   //list[0] for file playing, list[1] for display

    void loop();
};


Player::Player()
{
    p = new PlayerPrivate;
}


void Player::loop()
{
    p->psys->loop2();
    p->info->loop2();
    p->loop();

    auto page = *page_ptr(ui_page);
    page->gui_loop();

    p->gui->loop();
}


Page ** Player::page_ptr(int page_num)
{
    switch (page_num)
    {
    case PAGE_INFO:     return (Page**)&p->info;
    case PAGE_SYS:      return (Page**)&p->psys;
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


void Player::set_sound(Sound * sound)
{
    p->sound = sound;
}


void Player::set_playlist(PlaylistType pl, void * playlist)
{
    p->list[pl] = (Playlist *)playlist;
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
    sound->pause();
//    controls_pause();
}


void Player::unfreeze()
{
    sound->resume();
//    controls_resume();
}


//###############################################################
bool Player::fav_switch(int fav_num, bool init)
{
    DEBUG("switch to fav %d, %d\n", fav_num, init);

    if (!init)
    {
        if (sound->is_playing())
            filepos = sound->current_time();

        sound->stop_cmd();
        sound->wait();

        prefs_save_now(need_save_current_file);

        prev_fav_num = cur_fav_num;
        prefs_save_main(fav_num, prev_fav_num, sys.sd_free_mb);
    }

    fav_num = clamp1(fav_num, FAV_MAX);
    cur_fav_num = fav_num;

    char fav_path[PATHNAME_MAX_LEN] = {0};
    prefs_load_data(fav_num, fav_path, sizeof(fav_path));
    DEBUG("fav path: %s\n", fav_path);
    
    p->list[PLAYING]->set_root(fav_path);
    p->list[LIST]->copy_from(p->list[PLAYING]);

    p->files->box(p->list[LIST]->filecnt);
    p->dirs->box(p->list[LIST]->dircnt);

    DEBUG("dircnt: %d\n", p->list[LIST]->dircnt);

    p->info->update();
    p->info->alive(false);
    p->info->gain(false);
    p->info->index("");
    
    playstack_init();
    play_file_num(next_file, FAIL_NEXT);

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


void Player::restart()
{
    fav_switch(cur_fav_num, false);
}


const char * Player::cur_fav_path()
{
    return p->list[PLAYING]->root_path.c_str();
}


int Player::cur_dir(PlaylistType pl)
{
    return p->list[pl]->curdir;
}


int Player::cur_file(PlaylistType pl)
{
    return p->list[pl]->curfile;
}


int Player::cur_level(PlaylistType pl)
{
    return p->list[pl]->level;
}


int Player::file_random()
{
    return random(1, p->list[PLAYING]->filecnt+1);
}


int Player::filecnt()
{
    return p->list[PLAYING]->filecnt;
}


bool Player::set_dir(PlaylistType pl, int dir_num)
{
    return p->list[pl]->find_dir(dir_num);
}


bool Player::set_file(PlaylistType pl, int file_num)
{
    return p->list[pl]->find_file(file_num);
}


bool Player::file_is_dir(PlaylistType pl, int file_num)
{
    return p->list[pl]->file_is_dir(file_num);
}


size_t Player::file_name(PlaylistType pl, int file_num, char * dst, int len)
{
    return p->list[pl]->file_name(file_num, dst, len);
}


size_t Player::dir_name(PlaylistType pl, int file_num, char * dst, int len)
{
    return p->list[pl]->file_dirname(file_num, dst, len);
}


//###############################################################
void Player::play_file_num(int num, int updown)
{
    num = clamp1(num, p->list[PLAYING]->filecnt);
    next_file = num;
    next_updown = updown;
    need_play_next_file = NEED_START;
}


void Player::play_file_up()
{
    play_file_num(p->list[PLAYING]->curfile - 1, FAIL_PREV);
}


void Player::play_file_down()
{
    play_file_num(p->list[PLAYING]->curfile + 1, FAIL_NEXT);
}


void Player::play_file_random()
{
    int n = file_random();
    while (n == p->list[PLAYING]->curfile && p->list[PLAYING]->filecnt > 1)
    {
        n = file_random();
    }

    play_file_num(n, FAIL_RANDOM);
}


int sound_errors = 0;
//todo: remove player, move to PlayerPrivate fields
void PlayerPrivate::loop()
{
    static int retry = 0;
    static int num = 0;
    static int updown = FAIL_NEXT;
    char tmp[QUEUE_MSG_SIZE];

    if (!player->filecnt())
        return;

    if (!player->need_play_next_dir && !player->need_play_next_file)
        return;

    if (sound->state == SOUND_STARTING)
        return;

    if (player->need_play_next_file != NEED_CONT)
    {
        if (sound->state == SOUND_PLAYING || sound->state == SOUND_PAUSED)
        {
            // DEBUG("-clear\n");
            info->clear();
            sound->stop_cmd();
            sound->wait();
        }
    }

    if (player->need_play_next_dir)
    {
        // DEBUG("-next dir\n");
        player->need_play_next_dir = false;
        player->set_dir(PLAYING, player->next_dir);
        player->next_file = player->cur_file(PLAYING);
        player->need_play_next_file = NEED_START;
    }

    if (player->need_play_next_file == NEED_START)
    {
        // DEBUG("-need start\n");
        retry = player->filecnt();
        num = player->next_file;
        updown = player->next_updown;
        player->need_play_next_file = NEED_CONT;
    }
    else
    {
        if (sound->state == SOUND_PLAYING)
        {
            // DEBUG("-playing\n");
            playstack_push(num);
            player->need_play_next_file = NEED_NOT;
            return;
        }

        num = (updown == FAIL_RANDOM) ? player->file_random() : num + updown;
    }

    if (sound->state == SOUND_ERROR)
    {
        // DEBUG("-error\n");
        sound_errors += 1;
        retry--;
    }

    if (retry == 0)
    {
        sys.error("File retry count=0");
        player->need_play_next_file = NEED_NOT;
        return;
    }

    num = clamp1(num, player->filecnt());
    DEBUG("Trying to play %d...\n", num);
    int level = playstack_is_instack(num);
    if (!((player->filecnt() <= PLAYSTACK_LEVELS) || (level == PLAYSTACK_NOT_IN_STACK) || (updown != FAIL_RANDOM)))
    {
        DEBUG("File is in stack %d\n", level);
        retry--;
        return;
    }

    if (!player->set_file(PLAYING, num))
    {
        // DEBUG("-not set\n");
        char tmp[QUEUE_MSG_SIZE];
        snprintf(tmp, sizeof(tmp)-1, "File %d not found", num);
        sys.error(tmp);
        player->need_play_next_file = NEED_NOT;
        return;
    }

    if (player->file_is_dir(PLAYING, num))
    {
        // DEBUG("-is dir\n");
        retry--;
        return;
    }

    // DEBUG("-ok\n");

    snprintf(tmp, sizeof(tmp)-1, "%i", num);
    info->index(tmp);

    char dirname[PATHNAME_MAX_LEN] = "";
    int x = player->dir_name(PLAYING, num, dirname, sizeof(dirname));
    info->path(dirname, "");    //todo: remove root

    char filename[PATHNAME_MAX_LEN] = "";
    player->file_name(PLAYING, num, filename, sizeof(filename));
    info->file(filename);

    char filepath[PATHNAME_MAX_LEN] = "";
    strlcpy(filepath, dirname, sizeof(filepath));
    if (x > 1) filepath[x++] = '/';
    strlcpy(&filepath[x], filename, sizeof(filepath)-x);
    
    player->filepos = 0;
    prefs_save_delayed(need_save_current_file);
    sound->play_cmd(filepath);
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
            n = p->list[PLAYING]->curfile;
        play_file_num(n, FAIL_RANDOM);
    }
    else
    {
        play_file_up();
    }
}


void Player::play_dir_next()
{
    next_dir = p->list[PLAYING]->curdir + 1;
    next_updown = FAIL_NEXT;
    need_play_next_dir = true;
}


void Player::play_dir_prev()
{
    next_dir = p->list[PLAYING]->curdir - 1;
    next_updown = FAIL_PREV;
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
    sound->stop_cmd();
    sound->wait();
    prefs_erase_all();
}


void Player::toggle_pause()
{
    if (!sound->is_playing())
    {
        sound->resume();
        return;
    }
        
    sound->pause();
    filepos = sound->current_time();
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
    p->gui->set_page(ui_page);

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

