#include <nvs_flash.h>
#include "Preferences.h"

#include "debug.h"

#include "globals.h"
#include "controls.h"
#include "player.h"

#include "prefs.h"


//todo: prefs.freeEntries();

//###############################################################
// Preferences
//###############################################################
static Preferences prefs;

#define NO_FAV 0
static int curfav = NO_FAV;

static const char *prefs_file_main    = "main";
static const char *prefs_key_curfav   = "curfav";
static const char *prefs_key_prevfav  = "prevfav";
static const char *prefs_key_controls = "controls";
static const char *prefs_key_sd_free  = "sd_free";

static const char *prefs_file_dir    = "fav";
static const char *prefs_key_path    = "path";
static const char *prefs_key_volume  = "volume";
static const char *prefs_key_curfile = "curfile";
static const char *prefs_key_filepos = "filepos";
static const char *prefs_key_shuffle = "shuffle";
static const char *prefs_key_repeat  = "repeat";

//flags
uint32_t save_time;
bool need_save_current_file = false;
bool need_save_volume = false;
bool need_save_repeat = false;
bool need_save_shuffle = false;
bool initialized = false;
//bool need_save_file_pos = false;


//###############################################################
void prefs_erase_all()
{
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init(); // initialize the NVS partition.            
    esp_restart();
}


//###############################################################
static void prefs_close_fav()
{
    prefs.end();
    curfav = NO_FAV;
}


bool prefs_open_fav(int fav_num)
{
    if (curfav == fav_num)
        return true;

    prefs_close_fav();

    String root_file(prefs_file_dir);
    root_file += String(fav_num);
    prefs.begin(root_file.c_str());
    curfav = fav_num;

    return true;
}


int prefs_get_path(int fav_num, char * path, int len)
{
    int tmpfav = curfav;

    prefs_open_fav(fav_num);

    int size = 0;
    if (prefs.isKey(prefs_key_path)) {
        size = prefs.getString(prefs_key_path, path, len);
    }

    if (!size)
    {
        strcpy(path, "/");
        size = strlen(path);
    }
  
    prefs_open_fav(tmpfav);
    
    return size;
}


int prefs_set_path(int fav_num, const char * path)
{
    int tmpfav = curfav;

    prefs_open_fav(fav_num);
    int size = prefs.putString(prefs_key_path, path);

    prefs_open_fav(tmpfav);

    return size;
}


int prefs_load_data(int fav_num, char * path, int len)
{
    prefs_open_fav(fav_num);

    player->volume    = prefs.getInt (prefs_key_volume, 1);
    player->next_file = prefs.getInt (prefs_key_curfile, 1);
    player->shuffle   = prefs.getBool(prefs_key_shuffle, false);
    player->repeat    = prefs.getBool(prefs_key_repeat, false);
    player->filepos   = prefs.getInt (prefs_key_filepos, 0);
    int size = prefs_get_path(fav_num, path, len);

    DEBUG("Fav:%i Dir:%s File:%i Pos:%i Vol:%i Sh:%i Rep:%i\n", fav_num, path, player->next_file, player->filepos, player->volume, player->shuffle, player->repeat);
    
    return size;
}


void prefs_load_main(int * curfav, int * prevfav, uint32_t * sd_free)
{
    prefs_close_fav();
    prefs.begin(prefs_file_main, false);

    *curfav = prefs.getInt(prefs_key_curfav, 1);
    *prevfav = prefs.getInt(prefs_key_prevfav, 1);
    *sd_free = prefs.getUInt(prefs_key_sd_free, 0);
    
    uint8_t arr[CONTROLS_PREFS_SIZE];
    if (prefs.getBytes(prefs_key_controls, arr, sizeof(arr)))
        controls_set_prefs(arr);

    prefs.end();
}


void prefs_save_main(int curfav, int prevfav, uint32_t sd_free)
{
    prefs_close_fav();
    prefs.begin(prefs_file_main, false);

    prefs.putInt(prefs_key_curfav, curfav);
    prefs.putInt(prefs_key_prevfav, prevfav);
    prefs.putUInt(prefs_key_sd_free, sd_free);

    uint8_t arr[CONTROLS_PREFS_SIZE];
    controls_get_prefs(arr);
    prefs.putBytes(prefs_key_controls, arr, sizeof(arr));

    prefs.end();

    DEBUG("prefs saved\n");
}


//###############################################################
// delayed save - prevent flash wearing
//###############################################################
void prefs_loop()
{
    if ((int32_t)(millis() - save_time) < 0)
        return;

    if (need_save_current_file)
    {
        need_save_current_file = false;
        prefs.putInt(prefs_key_curfile, player->cur_file(PLAYING));
        prefs.putInt(prefs_key_filepos, player->filepos);
        DEBUG("File %i:%i saved\n", player->cur_file(PLAYING), player->filepos);
    }

    if (need_save_repeat)
    {
        need_save_repeat = false;
        prefs.putBool(prefs_key_repeat, player->repeat);
        DEBUG("Repeat %i saved\n", player->repeat);
    }

    if (need_save_shuffle)
    {
        need_save_shuffle = false;
        prefs.putBool(prefs_key_shuffle, player->shuffle);
        DEBUG("Shuffle %i saved\n", player->shuffle);
    }

    if (need_save_volume)
    {
        need_save_volume = false;
        prefs.putInt(prefs_key_volume, player->volume);
        DEBUG("Volume %d saved\n", player->volume);
    }
}


void prefs_save_delayed(bool &flag)
{
//    DEBUG("Save delayed\n");
    const int save_delay_ms = 5000;
    flag = true;
    save_time = millis() + save_delay_ms;
}


void prefs_save_now(bool &flag)
{
//    DEBUG("Save now\n");
    flag = true;
    save_time = millis();
    prefs_loop();
}


int prefs_free(void)
{
    return prefs.freeEntries();
}
