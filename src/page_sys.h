#ifndef _PAGE_SYS_H_
#define _PAGE_SYS_H_

#include "player_ctrl.h"


class Sys
{
public:
    uint32_t sd_free_mb = 0;
    uint32_t gui_sys_freeheap;
    uint32_t gui_sys_minheap;
    //uint32_t gui_sys_largestheap;

    bool read_error = false;
};


extern CtrlPage * ctrl_page_sys;

uint32_t calc_sd_free_size();

extern Sys sys;

void page_sys_init();
void page_sys_set_update();
void page_sys_loop();


#endif // _PAGE_SYS_H_
