#ifndef _GLOBALS_H_
#define _GLOBALS_H_


#include "pinout.h"
#include "playlist.h"   // зачем это здесь???

//###############################################################
#define PATHNAME_MAX_LEN 256

#define FAIL_NEXT +1
#define FAIL_PREV -1
#define FAIL_RANDOM 0

//###############################################################
#define FAV_MAX 20

//###############################################################
extern bool need_play_next_dir;
extern bool need_play_next_file;
extern bool need_set_file_pos;

extern uint32_t save_time;
extern bool need_save_current_file;
extern bool need_save_volume;
extern bool need_save_repeat;
extern bool need_save_shuffle;
extern bool need_save_file_pos;

extern Playlist * fc;   //playing
extern Playlist * pl;   //list

#define QUEUE_MSG_SIZE 250
extern QueueHandle_t tag_queue;


//###############################################################

int dirs_file_num(int dirs_sel);


//###############################################################


#endif // _GLOBALS_H_
