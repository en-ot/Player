#ifndef _PREFS_H_
#define _PREFS_H_

//###############################################################
#define FAV_MAX 20

//###############################################################
void prefs_erase_all();

void prefs_save(int curfav, int prevfav, uint32_t sd_free);
void prefs_load(int * curfav, int * prevfav, uint32_t * sd_free);

int prefs_load_data(int fav_num, char * path, int len);
int prefs_get_path(int fav_num, char * path, int len);
int prefs_set_path(int fav_num, const char * path);

void prefs_save_now(bool &flag);
void prefs_save_delayed(bool &flag);

void prefs_loop();

int prefs_free(void);


//###############################################################

#endif // _PREFS_H_
