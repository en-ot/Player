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
extern Playlist * fc;   //playing
extern Playlist * pl;   //list

#define QUEUE_MSG_SIZE 250
extern QueueHandle_t tag_queue;

#endif // _GLOBALS_H_
