#ifndef _PAGE_SYS_H_
#define _PAGE_SYS_H_


class Sys
{
public:
    uint32_t sd_free_mb = 0;
    uint32_t gui_sys_freeheap;
    uint32_t gui_sys_minheap;
    //uint32_t gui_sys_largestheap;
};

uint32_t calc_sd_free_size();

void gui_sys_init();
void memory_loop();

extern Sys sys;

#endif // _PAGE_SYS_H_
