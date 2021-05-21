#include <nvs_flash.h>
#include "Preferences.h"

#include "globals.h"
#include "prefs.h"


//todo: prefs.freeEntries();

//###############################################################
// Preferences
//###############################################################
static Preferences prefs;

#define NO_FAV 0
static int curfav = NO_FAV;

static const char *prefs_file_main = "main";
static const char *prefs_key_curfav = "curfav";

static const char *prefs_file_dir = "fav";
static const char *prefs_key_path   = "path";
static const char *prefs_key_volume = "volume";
static const char *prefs_key_curfile = "curfile";
static const char *prefs_key_filepos = "filepos";
static const char *prefs_key_shuffle = "shuffle";
static const char *prefs_key_repeat = "repeat";

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


static bool prefs_open_fav(int fav_num)
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
    int size = prefs.getString(prefs_key_path, path, len);
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

    volume = prefs.getInt(prefs_key_volume, 1);
    next_file = prefs.getInt(prefs_key_curfile, 1);
    shuffle = prefs.getBool(prefs_key_shuffle, false);
    repeat = prefs.getBool(prefs_key_repeat, false);
    filepos = prefs.getInt(prefs_key_filepos, 0);
    int size = prefs_get_path(fav_num, path, len);

    Serial.printf("Fav:%i Dir:%s File:%i Pos:%i Vol:%i Sh:%i Rep:%i\n", fav_num, path, next_file, filepos, volume, shuffle, repeat);
    
    return size;
}


int prefs_load_curfav()
{
    prefs_close_fav();

    prefs.begin(prefs_file_main, false);
    int fav_num = prefs.getInt(prefs_key_curfav, 1);
    prefs.end();

    prefs_open_fav(fav_num);

    return fav_num;
}


void prefs_save_curfav(int fav_num)
{
    prefs_close_fav();

    prefs.begin(prefs_file_main, false);
    prefs.putInt(prefs_key_curfav, fav_num);
    prefs.end();

    prefs_open_fav(fav_num);
}


//###############################################################
// delayed save - prevent flash wearing
//###############################################################
void prefs_loop()
{
    if ((signed long)(millis() - save_time) < 0)
        return;

    if (need_save_current_file)
    {
        need_save_current_file = false;
        prefs.putInt(prefs_key_curfile, fc->curfile);
        prefs.putInt(prefs_key_filepos, filepos);
        Serial.printf("File %i:%i saved\n", fc->curfile, filepos);
    }

    if (need_save_repeat)
    {
        need_save_repeat = false;
        prefs.putBool(prefs_key_repeat, repeat);
        Serial.printf("Repeat %i saved\n", repeat);
    }

    if (need_save_shuffle)
    {
        need_save_shuffle = false;
        prefs.putBool(prefs_key_shuffle, shuffle);
        Serial.printf("Shuffle %i saved\n", shuffle);
    }

    if (need_save_volume)
    {
        need_save_volume = false;
        prefs.putInt(prefs_key_volume, volume);
        Serial.printf("Volume %d saved\n", volume);
    }
}


void prefs_save_delayed(bool &flag)
{
    const int save_delay_ms = 5000;
    flag = true;
    save_time = millis() + save_delay_ms;
}


void prefs_save_now(bool &flag)
{
    flag = true;
    save_time = millis();
    prefs_loop();
}

