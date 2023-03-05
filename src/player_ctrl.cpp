#include <Arduino.h>
#include "debug.h"

#include "player.h"
#include "player_ctrl.h"

#include "page_sys.h"
#include "page_dirs.h"

#include "globals.h"


bool process_key(int key, CtrlPage *page)
{
    player->next_file = player->cur_file(PLAYING) + 1;

    switch (key)
    {
    case 'q':   page->b1_short(); break;
    case 'Q':   page->b1_long(); break;
    case 'a':   page->b2_short(); break;
    case 'A':   page->b2_long(); break;
    case 'z':   page->b3_short(); break;
    case 'Z':   page->b3_long(); break;
    case 'w':   page->vol(1); break;
    case 's':   page->vol(-1); break;
    case 'x':   page->vol_short(); break;
    case 'X':   page->vol_long(); break;
    case 'e':   page->seek(1); break;
    case 'd':   page->seek(-1); break;
    case 'c':   page->seek_short(); break;
    case 'C':   page->seek_long(); break;

//    case 'd':   player->play_dir_next();    break;
//    case 'a':   player->play_dir_prev();    break;
//    case 'e':   player->toggle_pause();     break;
//    case 'q':   player->fav_prev();   break;

    case 'R':   player->reset_to_defaults();    break;
    // case 'c':   controls_calibrate(1);          break;
    // case 'v':   controls_calibrate(2);          break;

    case 'r':   ESP.restart(); break;

    case 'F':
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
        return process_key(key, this);

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






