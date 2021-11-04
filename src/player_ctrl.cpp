#include <Arduino.h>
#include "debug.h"

#include "player.h"
#include "player_ctrl.h"

#include "page_sys.h"

#include "globals.h"


bool process_key(int key)
{
    player->next_file = fc->curfile + 1;

    switch (key)
    {
    case 'd':   player->play_dir_next();    break;
    case 'a':   player->play_dir_prev();    break;
    case 'e':   player->toggle_pause();     break;
    case 'q':   player->play_root_prev();   break;

    case 'f':   player->reset_to_defaults();    break;
    // case 'c':   controls_calibrate(1);          break;
    // case 'v':   controls_calibrate(2);          break;

    case 'r':
    {
        // if (SD.begin())
        // {
        //     filectrl_rewind();
        //     DEBUG("Error Reset!\n");
        // }
        break;
    }

    case 'm':
    {
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        heap_caps_print_heap_info(MALLOC_CAP_EXEC);
        heap_caps_print_heap_info(MALLOC_CAP_32BIT);
        break;
    }

    default:
        return false;

    }

    return true;
}


class CtrlPageInfo : public CtrlPage
{
public:
    void b1_short()         {       DEBUG("!\n");
        player->change_page();}
    bool vol(int change)    {return player->change_volume(change);}
    void vol_short()        {       player->play_file_next();}
    void vol_long()         {       player->play_file_prev();}
    bool seek(int by)       {return player->file_seek(by);}
    void seek_short()       {       player->toggle_pause();}
    void seek_long()        {       player->play_root_next();}
    void b1_long()          {       player->play_file_down();}
    void b2_long()          {       player->toggle_shuffle();}
    void b2_short()         {       player->play_dir_prev();}
    void b3_long()          {       player->toggle_repeat();}
    void b3_short()         {       player->play_dir_next();}
} ctrl_page_info;


class CtrlPageFiles : public CtrlPage
{
    void b1_short()         {       player->change_page();}
    bool vol(int change)    {return player->files_pgupdn(change);}
    void vol_short()        {       player->files_goto_curfile();}
    bool seek(int by)       {return player->files_seek(by);}
    void seek_long()        {       player->files_set_fav();}
    void seek_short()       {       player->files_play_sel();}
    void b2_short()         {       player->files_dir_prev();}
    void b3_short()         {       player->files_dir_next();}
} ctrl_page_files;


class CtrlPageDirs : public CtrlPage
{
    void b1_short()         {       player->change_page();}
    bool vol(int change)    {return player->dirs_pgupdn(change);}
    void vol_short()        {       player->dirs_goto_curdir();}
    bool seek(int by)       {return player->dirs_seek(by);}
    void seek_long()        {       player->dirs_set_fav();}
    void seek_short()       {       player->dirs_play_sel();}
} ctrl_page_dirs;


class CtrlPageFav : public CtrlPage
{
    void b1_short()         {       player->change_page();}
    void b2_short()         {       player->sys_page();}
    bool vol(int change)    {return player->fav_pgupdn(change);}
    void vol_short()        {       player->fav_goto_curfav();}
    bool seek(int by)       {return player->fav_seek(by);}
    void seek_long()        {       player->fav_reset();}
    void seek_short()       {       player->fav_set_num();}
} ctrl_page_fav;


bool CtrlPage::input(PlayerInputType type, int key)
{
    if (type == I_SEEK1)
        return vol(key);
    
    if (type == I_SEEK2)
        return seek(key);


    if (type == I_KEY)
        return process_key(key);

    if (type != I_BUTTON) 
        return false;



    switch (key)
    {
    case BTN_SEEKSHORT: seek_short();   break;
    case BTN_SEEKLONG:  seek_long();    break;
    case BTN_VOLSHORT:  vol_short();    break;
    case BTN_VOLLONG:   vol_long();     break;
    case BTN_B1LONG:    b1_long();      break;
    case BTN_B1SHORT:   b1_short();     break;
    case BTN_B2LONG:    b2_long();      break;
    case BTN_B2SHORT:   b2_short();     break;
    case BTN_B3LONG:    b3_long();      break;
    case BTN_B3SHORT:   b3_short();     break;
    default:            return false;
    }
    return true;
}


CtrlPage * ctrl_pages[PAGE_MAX];
 
void player_ctrl_init()
{
    ctrl_pages[PAGE_INFO]  = &ctrl_page_info;
    ctrl_pages[PAGE_FAV]   = &ctrl_page_fav;
    ctrl_pages[PAGE_FILES] = &ctrl_page_files;
    ctrl_pages[PAGE_DIRS]  = &ctrl_page_dirs;
    ctrl_pages[PAGE_SYS]   = ctrl_page_sys;
}


bool player_ctrl_input(int ui_page, PlayerInputType type, int key)
{
    CtrlPage * page = ctrl_pages[ui_page];
    return page->input(type, key);
    // DEBUG_DUMP32(ctrl_pages, 5, 5);
}




