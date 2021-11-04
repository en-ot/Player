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


class SysPrivate
{
public:
    int sel = 1;
    void box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb, GSLC_CB_TICK tick_cb);

    bool seek(int by);
    bool vol(int by);
    void vol_short();
    void b2_short();
    void b3_short();
};

SysPrivate gui_sys;


class CtrlPageSys : public CtrlPage
{
    void b1_short()         {       player->change_page();}
    bool vol(int change)    {return gui_sys.vol(change);}
    void vol_short()        {       gui_sys.vol_short();}
    bool seek(int by)       {return gui_sys.seek(by);}
    void b2_short()         {       gui_sys.b2_short();}
    void b3_short()         {       gui_sys.b3_short();}
} ctrl_page_sys_;

CtrlPage * ctrl_page_sys = &ctrl_page_sys_;


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


//###############################################################
// page:sys
//###############################################################
gslc_tsElem                 sys_elem[SYS_ELEM_MAX];
gslc_tsElemRef              sys_ref[SYS_ELEM_MAX];

gslc_tsXListbox             sys_box_elem;
gslc_tsElemRef*             sys_box_ref     = NULL;

gslc_tsXSlider              sys_slider_elem;
gslc_tsElemRef*             sys_slider_ref  = NULL;


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
            snprintf(pStrItem, nStrItemLen, "Free Heap: %d", sys.gui_sys_freeheap);
            break;

        case 6:
            snprintf(pStrItem, nStrItemLen, "Min Free Heap: %d", sys.gui_sys_minheap);
            break;

        case 7:
            snprintf(pStrItem, nStrItemLen, "microSD free: %u MB", sys.sd_free_mb);
            break;
    }
    return true;
}


void page_sys_init()
{
    gslc_PageAdd(&gslc, PAGE_SYS, sys_elem, SYS_ELEM_MAX, sys_ref, SYS_ELEM_MAX);
    sys_box_ref   = create_listbox(PAGE_SYS, SYS_BOX_ELEM,    &sys_box_elem,    SYS_BACK_COL);
    sys_slider_ref = create_slider(PAGE_SYS, SYS_SLIDER_ELEM, &sys_slider_elem, SYS_BACK_COL);
    gui_sys.box(7, page_sys_get_item, 0);//sys_tick);
}


void page_sys_set_update()
{
    // DEBUG("Sys set update\n");
    gslc_ElemSetRedraw(&gslc, sys_box_ref, GSLC_REDRAW_FULL);
}


void SysPrivate::box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb, GSLC_CB_TICK tick_cb)
{
    sys_box_elem.pfuncXGet = cb;
    sys_box_elem.nItemCnt = cnt;
    sys_box_elem.bNeedRecalc = true;
    gslc_ElemSetTickFunc(&gslc, sys_box_ref, tick_cb);
}


bool SysPrivate::seek(int by)
{
    if (!by)
        return false;
    sel = box_goto(sys_box_ref, sys_slider_ref, sel-1-by, false) + 1;
    return true;
}


bool SysPrivate::vol(int by)
{
    if (!by)
        return false;

    if (sel == 3)
    {
        network_reconnect(by > 0 ? 1 : 0);
    }
    return true;
}


void SysPrivate::vol_short()
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


void SysPrivate::b2_short()
{
    if (sel == 1)
    {
        controls_calibrate(1);
    }
}


void SysPrivate::b3_short()
{
    if (sel == 1)
    {
        controls_calibrate(2);
    }
}


void page_sys_loop()
{
    static uint32_t t0 = 0;
//    static bool need_print = true;
    uint32_t t = millis();

    if ((int32_t)(t - t0) < 1000)
        return;

    t0 = t;

    uint32_t caps = MALLOC_CAP_DEFAULT;
    sys.gui_sys_freeheap = heap_caps_get_free_size(caps);
    sys.gui_sys_minheap =  heap_caps_get_minimum_free_size(caps);
    //gui_sys_largestheap = heap_caps_get_largest_free_block(caps);
    page_sys_set_update();

//    Serial.printf("free %d, min %d\n", gui_sys_freeheap, gui_sys_minheap);

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
//         // // gui_sys_freeheap = heap_caps_get_free_size(caps);
//         // // gui_sys_minheap =  heap_caps_get_minimum_free_size(caps);
//         // // gui_sys_largestheap = heap_caps_get_largest_free_block(caps);
//         // portENABLE_INTERRUPTS();
        
//         gui->set_update();
//         // DEBUG("Free heap: %d\n", freeheap);
//         t0 = t;
//     }
//     return true;
// }

