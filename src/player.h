// Player must contain all business logic, but no depenencies on components... maybe later...

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "globals.h"

#include "gui.h"
#include "player_input.h"
#include "player_ctrl.h"

#include "playlist.h"


class Page
{
public:
    virtual void init()     {};
    virtual void gui_loop() {};
    virtual void update()   {};
    virtual void activate() {};

    CtrlPage * ctrl = nullptr;
};


class Player
{
public:
    Player();
    void set_gui(Gui * gui);
    void loop();
    void update();
    bool input(PlayerInputType type, int key);

    int ui_page = -1;
    void set_page(int page_num, Page * page);
    Page ** page_ptr(int page_num);
    void page_change(int page);
    void page_next();

    void reset_to_defaults();
    void freeze();
    void unfreeze();

    int cur_fav_num;
    int prev_fav_num;
    bool fav_switch(int fav_num, bool init);
    void fav_next();
    void fav_prev();
    void restart();
    // int fav_sel();
    void fav_set(const char * path);

    const char * cur_fav_path();
    void set_playlist_playing(void * playlist);
    int cur_playing_dir();
    int cur_playing_file();
    int file_random();
    int filecnt();
    void set_playing_dir(int dir_num);
    bool set_playing_file(int file_num);
    bool playing_file_is_dir(int file_num);
    size_t playing_file_name(int file_num, char * dst, int len);
    size_t playing_dir_name(int file_num, char * dst, int len);

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

    Playlist * list;   //playlist for display

private:
    class PlayerPrivate * p = nullptr;
    Playlist * playing;   //playlist for file playing
};

extern Player * player;


#endif // _PLAYER_H_
