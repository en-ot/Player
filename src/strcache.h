#ifndef _STRCACHE_H_
#define _STRCACHE_H_

#include <Arduino.h>

//TODO: from other files
#define XLISTBOX_MAX_STR 70


#define CACHE_MISS -1
#define CACHE_EMPTY 0

typedef struct
{
    int key;
    int access;
    uint32_t flags;
    char txt[XLISTBOX_MAX_STR];
} CacheLine;

typedef struct
{
    int cnt;
    int access;
    CacheLine * lines;
} ListboxCache;


void cache_init(ListboxCache * cache);
int cache_get_item(ListboxCache * cache, int key);
void cache_put_item(ListboxCache * cache, int key, char * buf, int flags);


#endif // _STRCACHE_H_
