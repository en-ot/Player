#ifndef _PLAYER_H_
#define _PLAYER_H_


#include "player_input.h"

// Player must contain all business logic, but no depenencies on components

class PlayerGui
{
public:
    PlayerGui();

private:

};


class Player
{
public:
    Player();
    void set_gui(PlayerGui * gui);
    
    bool input(PlayerInputType type, int key);

private:
    PlayerGui * _gui = nullptr;

};


//temp
bool files_goto_curfile();
bool dirs_goto_curdir();
bool fav_goto_curfav();

bool play_file_next();
int file_random();

bool play_dir_next();
bool play_dir_prev();
bool play_root_prev();


#endif // _PLAYER_H_