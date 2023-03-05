//###############################################################
// page:sys
//###############################################################

#include <Arduino.h>
#include "debug.h"

#include "SD_Libs.h"

#include "controls.h"
#include "globals.h"
#include "network.h"
//#include "sound.h"
#include "prefs.h"
#include "player.h"
#include "firmware.h"

#include "gui.h"
#include "gui_common.h"
#include "gui_sys.h"

#include "page_info.h"

#include "page_sys.h"


Sys sys;

#define STEPS_TOTAL 10
uint32_t step_t0 = 0;

#define PAGE_SYS_LINES  9
#define LINE_VERSION    1
#define LINE_DATE       2
#define LINE_IP         3
#define LINE_FREEPREFS  4
#define LINE_FREEHEAP   5
#define LINE_MINHEAP    6
#define LINE_FREESD     7
#define LINE_DEBUG      8
#define LINE_FTP        9


class SysPrivate
{
public:
    PageInfo * page = nullptr;
    Gui * gui;
};


Sys::Sys()
{
    p = new SysPrivate;
}


void Sys::set_page(PageInfo * page, Gui * gui)
{
    p->page = page;
    p->gui = gui;
}


void Sys::step_end(int step_num)
{
    int32_t t = millis();
    p->page->step_progress(step_num, STEPS_TOTAL);
    p->gui->loop();
    DEBUG("Step %d end (%dms)\n", step_num, (int)(t - step_t0));
}


void Sys::step_begin(const char * step_name)
{
    DEBUG("Init step: %s\n", step_name);
    if (p->page)
    {
        p->page->step_begin(step_name);
    }
    step_t0 = millis();
}


void Sys::step_progress(uint32_t pos, uint32_t total)
{
    p->page->step_progress(pos, total);
}


void Sys::error(const char * err_text)
{
    p->page->error(err_text);
    DEBUG("Error: %s\n", err_text);
}


void Sys::message(const char * message)
{
    p->page->message(message);
}


void Sys::net(int mode)
{
    if (p->page->net(mode))
        player->update();
}




//#####################################################################################################
class PageSysPrivate
{
public:
    int sel = LINE_VERSION;
    void box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb, GSLC_CB_TICK tick_cb);
    int go_to(int line);

    bool seek(int by);
    bool vol(int by);
    void vol_short();
    void b2_short();
    void b3_short();

    gslc_tsElem                 elem[SYS_ELEM_MAX];
    gslc_tsElemRef              ref[SYS_ELEM_MAX];

    gslc_tsXListbox             box_elem;
    gslc_tsElemRef*             box_ref     = NULL;

    gslc_tsXSlider              slider_elem;
    gslc_tsElemRef*             slider_ref  = NULL;

    Gui * gui;
    GuiPrivate * p;
    gslc_tsGui * s;
};


class PageSysCtrl : public CtrlPage
{
public:
    void b1_short()         {       player->page_next();}
    bool vol(int change)    {return g->vol(change);}
    void vol_short()        {       g->vol_short();}
    bool seek(int by)       {return g->seek(by);}
    void b2_short()         {       g->b2_short();}
    void b3_short()         {       g->b3_short();}

    PageSysPrivate * g;
    PageSys * p;
};


//###############################################################
#define SECTOR_SIZE 512
#define MBYTES (1024*1024)

uint32_t calc_sd_free_size()
{
    uint64_t lFree = SD.vol()->freeClusterCount();
    lFree *= SD.vol()->sectorsPerCluster() * SECTOR_SIZE;
    sys.sd_free_mb = lFree / MBYTES;
    DEBUG("microSD free size: %u MB\n", sys.sd_free_mb);
    return sys.sd_free_mb;
}


bool page_sys_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    //char str[50] = "";
    int pos = 0;
    //char * ptr = str;
    int16_t line = nItem+1;

    switch (line)
    {
        case LINE_VERSION:
            snprintf(pStrItem, nStrItemLen, "Version: %s", firmware_version());
            break;
        
        case LINE_DATE:
            pos = strlcpy(pStrItem, "Date: ", nStrItemLen);
            firmware_datetime(&pStrItem[pos], nStrItemLen-pos);
            break;
        
        case LINE_IP:
            pos = strlcpy(pStrItem, "IP: ", nStrItemLen);
            if ((network_connected()))
            {
                network_address(&pStrItem[pos], nStrItemLen-pos);
            }
            else
            {
                strlcat(pStrItem, "not connected", nStrItemLen);
            }
            break;

        case LINE_FREEPREFS:
            snprintf(pStrItem, nStrItemLen, "Free Prefs: %d", prefs_free());
            break;
        
        case LINE_FREEHEAP:
            snprintf(pStrItem, nStrItemLen, "Free Heap: %d", sys.freeheap);
            break;

        case LINE_MINHEAP:
            snprintf(pStrItem, nStrItemLen, "Min Free Heap: %d", sys.minheap);
            break;

        case LINE_FREESD:
            snprintf(pStrItem, nStrItemLen, "microSD free: %u MB", sys.sd_free_mb);
            break;

        case LINE_DEBUG:
            snprintf(pStrItem, nStrItemLen, "debug: %08X", debug_val);
            break;

        case LINE_FTP:
            extern uint32_t ftp_val;
            snprintf(pStrItem, nStrItemLen, "ftp: %d", ftp_val);
            break;

        //PAGE_SYS_LINES
    }
    return true;
}

void PageSys::activate()
{
    gslc_SetBkgndColor(g->s, SYS_BACK_COL);
}

PageSys::PageSys(Gui * gui)
{
    g = new PageSysPrivate;
    g->gui = gui;
    g->p = gui->p;
    g->s = &gui->p->gslc;

    auto c = new PageSysCtrl;
    c->g = g;
    c->p = this;
    ctrl = c;

    gslc_PageAdd(g->s, PAGE_SYS, g->elem, SYS_ELEM_MAX, g->ref, SYS_ELEM_MAX);
    g->box_ref    = g->p->create_listbox(PAGE_SYS, SYS_BOX_ELEM,    &g->box_elem,   SYS_BACK_COL);
    g->slider_ref = g->p->create_slider(PAGE_SYS, SYS_SLIDER_ELEM, &g->slider_elem, SYS_BACK_COL);
    g->box(PAGE_SYS_LINES, page_sys_get_item, 0);
}


void PageSys::update()
{
    // DEBUG("Sys set update\n");
    gslc_ElemSetRedraw(g->s, g->box_ref, GSLC_REDRAW_FULL);
}


void PageSysPrivate::box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb, GSLC_CB_TICK tick_cb)
{
    box_elem.pfuncXGet = cb;
    box_elem.nItemCnt = cnt;
    box_elem.bNeedRecalc = true;
    gslc_ElemSetTickFunc(s, box_ref, tick_cb);
}


void PageSys::show_ftp()
{
    g->go_to(LINE_FTP);
}


int PageSysPrivate::go_to(int line)
{
    return p->box_goto(box_ref, slider_ref, line-1, false) + 1;
}


bool PageSysPrivate::seek(int by)
{
    if (!by)
        return false;
    sel = go_to(sel-by);
    return true;
}


bool PageSysPrivate::vol(int by)
{
    if (!by)
        return false;

    if (sel == LINE_IP)
    {
        network_reconnect(by > 0 ? WIFI_MODE_HOME : WIFI_MODE_PHONE);
    }
    return true;
}


void PageSysPrivate::vol_short()
{
    if (sel == LINE_FREESD)
    {
        //sd_free_mb = 0;
        player->stop();
        calc_sd_free_size();
        prefs_save_main(player->cur_fav_num, player->prev_fav_num, sys.sd_free_mb);
        prefs_open_fav(player->cur_fav_num);
    }

    if (sel == LINE_IP)
    {
        network_reconnect(WIFI_MODE_STA0);
    }
}


void PageSysPrivate::b2_short()
{
    if (sel == LINE_VERSION)
    {
        controls_calibrate(1);
    }
}


void PageSysPrivate::b3_short()
{
    if (sel == LINE_VERSION)
    {
        controls_calibrate(2);
    }
}

//###############################################################
// error check
//###############################################################
#define ERROR_CHECK_INTERVAL 2000

void PageSys::loop2()
{
    static uint32_t last_time = 0;
    uint32_t t = millis();
    bool tick = false;
    if ((int32_t)(t - last_time) > ERROR_CHECK_INTERVAL)
    {
        tick = true;
        last_time = t;
    }

    if (tick && (sys.read_error || SD.card()->errorCode()))
    {
        last_time = millis();
        DEBUG(".");   
        if (SD.begin()) 
        {
            // filectrl_rewind();
            DEBUG("\n");                 
            sys.read_error = false;   
//            need_save_current_file = false;
            player->next_file = 1;
            player->need_play_next_file = true;
        }
    }
}


void PageSys::gui_loop()
{
    static uint32_t t0 = 0;
//    static bool need_print = true;
    uint32_t t = millis();

    if ((int32_t)(t - t0) < 1000)
        return;

    t0 = t;

    uint32_t caps = MALLOC_CAP_DEFAULT;
    sys.freeheap = heap_caps_get_free_size(caps);
    sys.minheap =  heap_caps_get_minimum_free_size(caps);
    //gui_sys_largestheap = heap_caps_get_largest_free_block(caps);
    update();

//    Serial.printf("free %d, min %d\n", freeheap, minheap);

    // uint32_t freeheap = ESP.getFreeHeap();
    // if (freeheap < 50000)
    // {
    //    need_print = true;
    // }
    
    // if (need_print)
    // {
    //     need_print = false;
    //     DEBUG("Memory free: %i\n", freeheap);
    // }
}



// bool sys_tick(void* pvGui, void* pvElemRef)
// {
//     static uint32_t t0 = 0;
//     uint32_t t = millis();
//     if ((int32_t)(t - t0) > 1000)
//     {
//         // portDISABLE_INTERRUPTS();
//         // uint32_t caps = MALLOC_CAP_DEFAULT;
//         // // freeheap = heap_caps_get_free_size(caps);
//         // // minheap =  heap_caps_get_minimum_free_size(caps);
//         // // gui_sys_largestheap = heap_caps_get_largest_free_block(caps);
//         // portENABLE_INTERRUPTS();
        
//         gui->set_update();
//         // DEBUG("Free heap: %d\n", freeheap);
//         t0 = t;
//     }
//     return true;
// }

