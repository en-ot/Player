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

    bool input(PlayerInputType type, int key);

    void set_page(int page);
    void next_page();

    bool change_volume(int change);
    void toggle_shuffle();
    void toggle_repeat();
    void toggle_pause();
    
    bool file_seek(int by);

    void play_file_num(int num, int updown);
    void play_file_up();
    void play_file_down();
    void play_file_next();
    void play_file_prev();
    void play_file_random();

    void play_dir_next();
    void play_dir_prev();

    void play_root_next();
    void play_root_prev();

    bool shuffle;
    bool repeat;
    int8_t volume;
    int8_t volume_old;

    int next_file;
    int next_updown = FAIL_NEXT;
    int next_dir;

    uint32_t filepos;
    int file_seek_by;

    int cur_fav_num;
    int prev_fav_num;

    int ui_page = PAGE_INFO;

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
