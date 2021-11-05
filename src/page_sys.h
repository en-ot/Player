#ifndef _PAGE_SYS_H_
#define _PAGE_SYS_H_

#include "player.h"
#include "player_ctrl.h"


uint32_t calc_sd_free_size();


class Sys
{
public:
    uint32_t sd_free_mb = 0;
    uint32_t freeheap;
    uint32_t minheap;
    //uint32_t largestheap;

    bool read_error = false;
};
extern Sys sys;


class PageSys : public Page
{
public:
    void init();
    void update();
    void gui_loop();
    void activate();

    void loop2();

    class PageSysPrivate * g;
};

extern PageSys page_sys;


#endif // _PAGE_SYS_H_
