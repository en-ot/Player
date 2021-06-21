#include <Arduino.h>

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

int cache_get_item(ListboxCache * cache, int key);
