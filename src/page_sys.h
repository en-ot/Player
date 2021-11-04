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


class PageSys
{
public:
    bool sys_seek(int by);
    bool sys_volupdn(int by);
    void sys_volshort();
    void cal_vol();
    void cal_seek();
};

extern CtrlPage * ctrl_page_sys;

uint32_t calc_sd_free_size();

void gui_sys_init();
void memory_loop();
void sys_control(int16_t line, int key);
//void gui_sys_update();

extern Sys sys;
extern PageSys page_sys;

#endif // _PAGE_SYS_H_
