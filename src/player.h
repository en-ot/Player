#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "gui.h"
#include "player_input.h"
#include "player_ctrl.h"

#include "globals.h"


int file_random();


// Player must contain all business logic, but no depenencies on components... maybe later...
class Player
{
public:
    Player();
    void set_gui(Gui * gui);
    void set_ctrl(int page, CtrlPage * ctrl);

    void reset_to_defaults();
    void freeze();
    void unfreeze();

    int cur_fav_num;
    int prev_fav_num;
    bool fav_switch(int fav_num, bool init);
    void fav_next();
    void fav_prev();
    void restart();

    bool input(PlayerInputType type, int key);

    int ui_page = PAGE_INFO;
    void set_page(int page);
    void next_page();

    int8_t volume;
    int8_t volume_old;
    bool change_volume(int change);

    bool shuffle;
    void toggle_shuffle();

    bool repeat;
    void toggle_repeat();
    void toggle_pause();

    bool need_set_file_pos = false;
    uint32_t filepos;
    int file_seek_by;
    bool file_seek(int by);

    bool need_play_next_file = false;
    int next_file;
    int next_updown = FAIL_NEXT;
    void play_file_num(int num, int updown);
    void play_file_up();
    void play_file_down();
    void play_file_next();
    void play_file_prev();
    void play_file_random();

    bool need_play_next_dir = false;
    int next_dir;
    void play_dir_next();
    void play_dir_prev();

private:
    Gui * _gui = nullptr;
    //CtrlPage * ctrl_pages[PAGE_MAX] = {0};
};


class Page
{
public:
    void init() {};
    void box(int dircnt) {};
    void goto_cur() {};
    void loop();
    void update();
    int sel();
};


extern Player * player;


#endif // _PLAYER_H_
