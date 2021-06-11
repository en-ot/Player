#ifndef _GLOBALS_H_
#define _GLOBALS_H_


#include "pinout.h"
#include "playlist.h"   // зачем это здесь???

//###############################################################
#define PATHNAME_MAX_LEN 256

//###############################################################
extern int file_seek_by;
extern uint32_t filepos;
extern bool shuffle;
extern bool repeat;
extern int8_t volume;

extern int cur_fav_num;

extern int8_t volume_old;

extern int next_file;

#define FAIL_NEXT +1
#define FAIL_PREV -1
#define FAIL_RANDOM 0
extern int next_updown;

extern int next_dir;

extern bool read_error;

extern bool need_play_next_dir;
extern bool need_play_next_file;

extern bool need_set_file_pos;

extern unsigned long save_time;
extern bool need_save_current_file;
extern bool need_save_volume;
extern bool need_save_repeat;
extern bool need_save_shuffle;
extern bool need_save_file_pos;

extern playlist * fc;   //playing
extern playlist * pl;   //list

#define QUEUE_MSG_SIZE 100
extern QueueHandle_t tag_queue;


//###############################################################
bool play_file_next();
int file_random();


//###############################################################


#endif // _GLOBALS_H_
