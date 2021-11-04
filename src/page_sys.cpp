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

#include "page_sys.h"


Sys sys;

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


void sys_control(int16_t line, int key)
{
    int16_t nItem = line-1;
    switch (nItem)
    {
    case 0:
        if (key == 4)
            controls_calibrate(1);
        if (key == 5)
            controls_calibrate(2);
        break;

    case 2:
        network_reconnect(key == 1);
        break;

    case 6:
        //sd_free_mb = 0;
        sound_stop();
        calc_sd_free_size();
        prefs_save_main(player->cur_fav_num, player->prev_fav_num, sys.sd_free_mb);
        prefs_open_fav(player->cur_fav_num);
        break;
    }
}


bool sys_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    //char str[50] = "";
    int pos = 0;
    //char * ptr = str;

    switch (nItem)
    {
        case 0:
            snprintf(pStrItem, nStrItemLen, "Version: %s", firmware_version());
            break;
        
        case 1:
            pos = strlcpy(pStrItem, "Date: ", nStrItemLen);
            firmware_datetime(&pStrItem[pos], nStrItemLen-pos);
            break;
        
        case 2:
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

        case 3:
            snprintf(pStrItem, nStrItemLen, "Free Prefs: %d", prefs_free());
            break;
        
        case 4:
            snprintf(pStrItem, nStrItemLen, "Free Heap: %d", sys.gui_sys_freeheap);
            break;

        case 5:
            snprintf(pStrItem, nStrItemLen, "Min Free Heap: %d", sys.gui_sys_minheap);
            break;

        case 6:
            snprintf(pStrItem, nStrItemLen, "microSD free: %u MB", sys.sd_free_mb);
            break;
    }
    return true;
}


void memory_loop()
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
    gui->sys_set_update();

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
        
//         gui->sys_set_update();
//         // DEBUG("Free heap: %d\n", freeheap);
//         t0 = t;
//     }
//     return true;
// }


void gui_sys_init()
{
    gui->sys_box(7, sys_get_item, 0);//sys_tick);
}


