#include <Arduino.h>
#include "debug.h"

#include "player.h"
#include "player_ctrl.h"

#include "page_sys.h"
#include "page_dirs.h"

#include "globals.h"


bool process_key(int key)
{
    player->next_file = fc->curfile + 1;

    switch (key)
    {
    case 'd':   player->play_dir_next();    break;
    case 'a':   player->play_dir_prev();    break;
    case 'e':   player->toggle_pause();     break;
    case 'q':   player->fav_prev();   break;

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
 
bool player_ctrl_input(int ui_page, PlayerInputType type, int key)
{
//    DEBUG_DUMP32(ctrl_pages, 5, 5);

    CtrlPage * page = ctrl_pages[ui_page];
    return page->input(type, key);
}




