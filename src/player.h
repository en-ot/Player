#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "gui.h"
#include "player_input.h"
#include "player_ctrl.h"


int file_random();


// Player must contain all business logic, but no depenencies on components... later...
class Player
{
public:
    Player();
    void set_gui(Gui * gui);
    
    bool input(PlayerInputType type, int key);

    void change_page();

    bool change_volume(int change);
    void toggle_shuffle();
    void toggle_repeat();
    void change_pause();

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

    bool files_seek(int by);
    bool files_pgupdn(int by);
    void files_dir_prev();
    void files_dir_next();
    void files_goto_curfile();
    void files_play_sel();
    void files_set_fav();

    bool fav_seek(int by);
    bool fav_pgupdn(int by);
    void fav_reset();
    void fav_set_num();
    void fav_goto_curfav();

    bool dirs_pgupdn(int by);
    bool dirs_seek(int by);
    void dirs_goto_curdir();
    void dirs_set_fav();
    void dirs_play_sel();

    void sys_page();;
    bool sys_seek(int by);
    bool sys_volupdn(int by);
    void sys_volshort();
    void cal_vol();
    void cal_seek();

private:
    Gui * _gui = nullptr;
    CtrlPage * ctrl_pages[PAGE_MAX] = {0};
};


//temp
//bool files_goto_curfile();
//bool dirs_goto_curdir();
//bool fav_goto_curfav();


//bool play_dir_next();
//bool play_dir_prev();
//bool play_root_prev();

//bool player_input(PlayerInputType type, int key);

extern Player * player;


#endif // _PLAYER_H_
