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
#define STEPS_TOTAL 10
uint32_t step_t0 = 0;

void begin(const char * step_name)
{
    DEBUG("Init step: %s\n", step_name);
    if (gui)
    {
        page_info.step_begin(step_name);
    }
    step_t0 = millis();
};


void end(int curstep)
{
    int32_t t = millis();
    page_info.step_progress(curstep, STEPS_TOTAL);
    gui->loop();
    DEBUG("Step %d end (%dms)\n", curstep, (int)(t - step_t0));
}


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

    // uint8_t *p1 = (uint8_t*)malloc(50000);
    // Serial.printf("%08X\n", (int)p1);
    // uint8_t *p2 = (uint8_t*)malloc(30000);
    // Serial.printf("%08X\n", (int)p2);

    player = new Player();

    begin("gui");
    gui = new Gui();
    player->set_gui(gui);

    page_info.init();
    page_files.init();
    //page_pic_init();
    page_fav.init();
    page_dirs.init();
    page_sys.init();
    player->change_page(PAGE_INFO);
    end(0);

    // free(p1);
    // free(p2);

    begin("sound");
    sound_setup();
    end(1);

    begin("queue");
    tag_queue = xQueueCreate(QUEUE_DEPTH, QUEUE_MSG_SIZE);
    end(2);

    begin("controls");
    controls_init(&player_input);
    end(3);

    begin("prefs");
    if (controls_defaults())
    {
        prefs_erase_all();
    }
    prefs_load_main(&player->cur_fav_num, &player->prev_fav_num, &sys.sd_free_mb);
    //prefs_open_fav(cur_fav_num);
    //Serial.printf("cur:%d prev:%d\n", cur_fav_num, prev_fav_num);
    end(4);

    begin("sdcard");
    if (!SD.begin()) 
    {
        page_info.error("SDcard init error");
        SD.initErrorHalt(); // SdFat-lib helper function
    }
    if (!sys.sd_free_mb)
    {
        calc_sd_free_size();
        prefs_save_main(player->cur_fav_num, player->prev_fav_num, sys.sd_free_mb);
    }
    end(5);

    begin("fav");
    page_fav.box();
    end(6);

    begin("filectrl");
    fc = new Playlist(DIRECT_ACCESS);
    end(7);
 
    begin("Playlist");
    pl = new Playlist(SAFE_ACCESS);
    end(8);

    begin("network");
    network_init();
    end(9);

    begin("start");
    player->fav_switch(player->cur_fav_num, true);
    end(10);
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


