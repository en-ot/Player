#ifndef _PREFS_H_
#define _PREFS_H_

//###############################################################
void prefs_erase_all();

void prefs_save_main(int curfav, int prevfav, uint32_t sd_free);
void prefs_load_main(int * curfav, int * prevfav, uint32_t * sd_free);

bool prefs_open_fav(int fav_num);
int prefs_load_data(int fav_num, char * path, int len);
int prefs_get_path(int fav_num, char * path, int len);
int prefs_set_path(int fav_num, const char * path);

void prefs_save_now(bool &flag);
void prefs_save_delayed(bool &flag);

void prefs_loop();

int prefs_free(void);


//###############################################################

#endif // _PREFS_H_
