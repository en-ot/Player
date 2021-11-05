#ifndef _PAGE_SYS_H_
#define _PAGE_SYS_H_

#include "player.h"
#include "player_ctrl.h"


uint32_t calc_sd_free_size();

class PageInfo;

class Sys
{
public:
    uint32_t sd_free_mb = 0;
    uint32_t freeheap;
    uint32_t minheap;
    //uint32_t largestheap;

    bool read_error = false;

    void set_page(PageInfo * page);

    void step_begin(const char * step_name);
    void step_end(int step_num);
    void step_progress(uint32_t pos, uint32_t total);
    void error(const char * err_text);
    void message(const char * message);
    void net(int mode);

private:
    PageInfo * page = nullptr;
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
