#ifndef _PLAYER_H_
#define _PLAYER_H_

// Player must contain all business logic, but no depenencies on components... maybe later...

int clamp1(int num, int cnt);   // from playlist.cpp


enum PlaylistType
{
    PLAYING = 0,
    LIST = 1,
};


class Page
{
public:
    virtual void init()     {};
    virtual void gui_loop() {};
    virtual void update()   {};
    virtual void activate() {};

    class CtrlPage * ctrl = nullptr;
};


class Player
{
public:
    Player();
    void loop();
    void update();
    bool input(int type, int key);

    void set_gui(class Gui * gui);
    void set_page(int page_num, Page * page);
    void set_playlist(PlaylistType pl, class Playlist * playlist);
    void set_sound(class Sound * sound);

    int ui_page = -1;
    Page ** page_ptr(int page_num);
    void page_change(int page);
    void page_next();

    void reset_to_defaults();
    void freeze();
    void unfreeze();

    int cur_fav_num = 1;
    int prev_fav_num = 1;
    bool fav_switch(int fav_num, bool init);
    void fav_next();
    void fav_prev();
    void restart();
    // int fav_sel();
    void fav_set(const char * path);

    const char * cur_fav_path();
    int file_random();
    int filecnt();

    int cur_dir(PlaylistType pl);
    int cur_file(PlaylistType pl);
    bool set_dir(PlaylistType pl, int dir_num);
    bool set_file(PlaylistType pl, int file_num);
    bool file_is_dir(PlaylistType pl, int file_num);
    size_t file_name(PlaylistType pl, int file_num, char * dst, int len);
    size_t dir_name(PlaylistType pl, int file_num, char * dst, int len);
    int cur_level(PlaylistType pl);

    int8_t volume = 1;
    int8_t volume_old = 1;
    bool change_volume(int change);

    bool shuffle = false;
    void toggle_shuffle();

    bool repeat = false;
    void toggle_repeat();

    bool need_set_file_pos = false;
    uint32_t filepos = 0;
    int file_seek_by = 0;
    bool file_seek(int by);

    int need_play_next_file = 0;
    int next_file = 1;
    int next_updown = 0;
    void play_file_num(int num, int updown);
    void play_file_up();
    void play_file_down();
    void play_file_next();
    void play_file_prev();
    void play_file_random();
    
    void toggle_pause();
    void stop();
    bool is_playing();
    bool is_gain();
    uint32_t current_time();
    uint32_t duration();

    int need_play_next_dir = 0;
    int next_dir = 1;
    void play_dir_next();
    void play_dir_prev();

private:
    class PlayerPrivate * p = nullptr;
};

extern Player * player;


#endif // _PLAYER_H_
