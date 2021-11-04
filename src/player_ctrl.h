#ifndef _PLAYER_CTRL_H_
#define _PLAYER_CTRL_H_


#include "player_input.h"

class CtrlPage
{
public:
    virtual bool vol(int change)    {return false;}
    virtual void vol_short()        {};
    virtual void vol_long()         {};

    virtual bool seek(int change)   {return false;}
    virtual void seek_short()       {};
    virtual void seek_long()        {};

    virtual void b1_short()         {};
    virtual void b1_long()          {};

    virtual void b2_short()         {};
    virtual void b2_long()          {};

    virtual void b3_short()         {};
    virtual void b3_long()          {};

    bool input(PlayerInputType type, int key);
};


void player_ctrl_init();
bool player_ctrl_input(int ui_page, PlayerInputType type, int key);


#endif // _PLAYER_CTRL_H_