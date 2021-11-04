#ifndef _PAGE_SYS_H_
#define _PAGE_SYS_H_

#include "player.h"
#include "player_ctrl.h"


class Sys
{
public:
    uint32_t sd_free_mb = 0;
    uint32_t freeheap;
    uint32_t minheap;
    //uint32_t largestheap;

    bool read_error = false;
};


extern CtrlPage * ctrl_page_sys;

uint32_t calc_sd_free_size();

extern Sys sys;


class PageSys : public Page
{
public:
    void init();
//    void box(int lines);
    void update();
    void loop();
};

extern PageSys page_sys;


//void page_sys_init();
//void page_sys_set_update();
void page_sys_loop();


#endif // _PAGE_SYS_H_
