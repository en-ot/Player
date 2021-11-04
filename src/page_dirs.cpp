#include <Arduino.h>
#include "debug.h"

#include "page_dirs.h"

class CtrlPageDirs : public CtrlPage
{
    void b1_short()         {       player->change_page();}
    bool vol(int change)    {return player->dirs_pgupdn(change);}
    void vol_short()        {       player->dirs_goto_curdir();}
    bool seek(int by)       {return player->dirs_seek(by);}
    void seek_long()        {       player->dirs_set_fav();}
    void seek_short()       {       player->dirs_play_sel();}
} ctrl_page_dirs_;


CtrlPage * ctrl_page_dirs = &ctrl_page_dirs_;
