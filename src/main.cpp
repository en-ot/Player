// builtin libs
#include <Arduino.h>

#include "SD_Libs.h"

#include "debug.h"

// project
#include "globals.h"
//#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "controls.h"
//#include "strcache.h"
#include "network.h"
//#include "firmware.h"

#include "page_info.h"
#include "page_files.h"
#include "page_dirs.h"
#include "page_fav.h"
#include "page_sys.h"

#include "playlist.h"
#include "player.h"
Player * player;


//###############################################################
// Setup
//###############################################################
void setup() 
{
    Serial.begin(115200);
    while (!Serial) 
    {       // Wait for USB Serial
        SysCall::yield();
    }
    DEBUG("\n");

    player = new Player();


    sys.step_begin("gui");
    auto gui = new Gui();
    player->set_gui(gui);

    auto page_info = new PageInfo(gui);
    page_info->init();
    player->set_page(PAGE_INFO, page_info);
    sys.set_page(page_info, gui);

    player->set_page(PAGE_FILES, new PageFiles(gui));
    
    auto page_fav = new PageFav(gui);
    player->set_page(PAGE_FAV, page_fav);

    player->set_page(PAGE_DIRS, new PageDirs(gui));

    player->set_page(PAGE_SYS, new PageSys(gui));

    //page_pic_init();

    player->page_change(PAGE_INFO);
    sys.step_end(0);


    sys.step_begin("queue");
    QueueHandle_t tag_queue = xQueueCreate(QUEUE_DEPTH, QUEUE_MSG_SIZE);
    page_info->set_queue(tag_queue);
    sys.step_end(1);


    sys.step_begin("sound");
    sound_setup(tag_queue);
    sys.step_end(2);


    sys.step_begin("controls");
    controls_init([](PlayerInputType type, int key){return player->input(type, key);});
    sys.step_end(3);


    sys.step_begin("prefs");
    if (controls_defaults())
    {
        prefs_erase_all();
    }
    prefs_load_main(&player->cur_fav_num, &player->prev_fav_num, &sys.sd_free_mb);
    //prefs_open_fav(cur_fav_num);
    //Serial.printf("cur:%d prev:%d\n", cur_fav_num, prev_fav_num);
    sys.step_end(4);


    sys.step_begin("sdcard");
    if (!SD.begin()) 
    {
        sys.error("SDcard init error");
        SD.initErrorHalt(); // SdFat-lib helper function
    }
    if (!sys.sd_free_mb)
    {
        calc_sd_free_size();
        prefs_save_main(player->cur_fav_num, player->prev_fav_num, sys.sd_free_mb);
    }
    sys.step_end(5);


    sys.step_begin("fav");
    page_fav->box();
    sys.step_end(6);


    sys.step_begin("filectrl");
    player->set_playlist(PLAYING, new Playlist(DIRECT_ACCESS));
    sys.step_end(7);
 

    sys.step_begin("Playlist");
    player->set_playlist(LIST, new Playlist(SAFE_ACCESS));
    sys.step_end(8);


    sys.step_begin("network");
    network_init();
    sys.step_end(9);


    sys.step_begin("start");
    player->fav_switch(player->cur_fav_num, true);
    sys.step_end(10);
}


//###############################################################
// Main Loop
//###############################################################
void loop()
{
    controls_loop();
    player->loop();
    prefs_loop();
    network_loop();
    yield();
    //vTaskDelay(1);      //avoid watchdog
}


