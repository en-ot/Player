// builtin libs
#include <Arduino.h>

#include "SD_Libs.h"

#include "debug.h"

// project
#include "globals.h"
#include "playstack.h"
#include "prefs.h"
#include "gui.h"
#include "sound.h"
#include "controls.h"
#include "strcache.h"
#include "network.h"
#include "firmware.h"

#include "page_info.h"
#include "page_files.h"
#include "page_dirs.h"
#include "page_fav.h"
#include "page_sys.h"

#include "player.h"

//###############################################################
// global objects
//###############################################################

Gui * gui;
Playlist * fc;   //playlist for file playing
Playlist * pl;   //playlist for display
Player * player;

#define QUEUE_DEPTH 20
QueueHandle_t tag_queue;


//###############################################################
// Init steps
//###############################################################
bool player_input(PlayerInputType type, int key)
{
    return player->input(type, key);
}


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
    gui = new Gui();
    player->set_gui(gui);

    auto page_info = new PageInfo(gui);
    page_info->init();
    player->set_page(PAGE_INFO, page_info);
    sys.set_page(page_info);

    page_files.init();
    page_fav.init();
    page_dirs.init();
    page_sys.init();
    //page_pic_init();

    player->change_page(PAGE_INFO);
    sys.step_end(0);

    sys.step_begin("sound");
    sound_setup();
    sys.step_end(1);

    sys.step_begin("queue");
    tag_queue = xQueueCreate(QUEUE_DEPTH, QUEUE_MSG_SIZE);
    sys.step_end(2);

    sys.step_begin("controls");
    controls_init(&player_input);
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
    page_fav.box();
    sys.step_end(6);

    sys.step_begin("filectrl");
    fc = new Playlist(DIRECT_ACCESS);
    sys.step_end(7);
 
    sys.step_begin("Playlist");
    pl = new Playlist(SAFE_ACCESS);
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
    gui->loop();
    prefs_loop();
    network_loop();
    yield();
    //vTaskDelay(1);      //avoid watchdog
}


