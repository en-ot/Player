//###############################################################
// page:sys
//###############################################################

#include <Arduino.h>
#include "debug.h"

#include "SD_Libs.h"

#include "controls.h"
#include "globals.h"
#include "network.h"
#include "sound.h"
#include "prefs.h"
#include "player.h"
#include "firmware.h"

#include "gui.h"
#include "gui_common.h"
#include "gui_sys.h"

#include "page_sys.h"


Sys sys;


class PageSysPrivate
{
public:
    int sel = 1;
    void box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb, GSLC_CB_TICK tick_cb);

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
};

PageSys page_sys;


class PageSysCtrl : public CtrlPage
{
public:
    void b1_short()         {       player->next_page();}
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
        case 1:
            snprintf(pStrItem, nStrItemLen, "Version: %s", firmware_version());
            break;
        
        case 2:
            pos = strlcpy(pStrItem, "Date: ", nStrItemLen);
            firmware_datetime(&pStrItem[pos], nStrItemLen-pos);
            break;
        
        case 3:
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

        case 4:
            snprintf(pStrItem, nStrItemLen, "Free Prefs: %d", prefs_free());
            break;
        
        case 5:
            snprintf(pStrItem, nStrItemLen, "Free Heap: %d", sys.freeheap);
            break;

        case 6:
            snprintf(pStrItem, nStrItemLen, "Min Free Heap: %d", sys.minheap);
            break;

        case 7:
            snprintf(pStrItem, nStrItemLen, "microSD free: %u MB", sys.sd_free_mb);
            break;
    }
    return true;
}


void PageSys::init()
{
    g = new PageSysPrivate;
    c = new PageSysCtrl;
    c->g = g;
    c->p = this;
    player->set_ctrl(PAGE_SYS, c);

    gslc_PageAdd(&gslc, PAGE_SYS, g->elem, SYS_ELEM_MAX, g->ref, SYS_ELEM_MAX);
    g->box_ref   = create_listbox(PAGE_SYS, SYS_BOX_ELEM,    &g->box_elem,    SYS_BACK_COL);
    g->slider_ref = create_slider(PAGE_SYS, SYS_SLIDER_ELEM, &g->slider_elem, SYS_BACK_COL);
    g->box(7, page_sys_get_item, 0);
}


void PageSys::update()
{
    // DEBUG("Sys set update\n");
    gslc_ElemSetRedraw(&gslc, g->box_ref, GSLC_REDRAW_FULL);
}


void PageSysPrivate::box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb, GSLC_CB_TICK tick_cb)
{
    box_elem.pfuncXGet = cb;
    box_elem.nItemCnt = cnt;
    box_elem.bNeedRecalc = true;
    gslc_ElemSetTickFunc(&gslc, box_ref, tick_cb);
}


bool PageSysPrivate::seek(int by)
{
    if (!by)
        return false;
    sel = box_goto(box_ref, slider_ref, sel-1-by, false) + 1;
    return true;
}


bool PageSysPrivate::vol(int by)
{
    if (!by)
        return false;

    if (sel == 3)
    {
        network_reconnect(by > 0 ? 1 : 0);
    }
    return true;
}


void PageSysPrivate::vol_short()
{
    if (sel == 7)
    {
        //sd_free_mb = 0;
        sound_stop();
        calc_sd_free_size();
        prefs_save_main(player->cur_fav_num, player->prev_fav_num, sys.sd_free_mb);
        prefs_open_fav(player->cur_fav_num);
    }
}


void PageSysPrivate::b2_short()
{
    if (sel == 1)
    {
        controls_calibrate(1);
    }
}


void PageSysPrivate::b3_short()
{
    if (sel == 1)
    {
        controls_calibrate(2);
    }
}

//###############################################################
// error check
//###############################################################
#define ERROR_CHECK_INTERVAL 2000

void check_loop()
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


void PageSys::loop()
{
    check_loop();
    
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

