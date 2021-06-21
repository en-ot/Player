#include <Arduino.h>
#include "debug.h"

#include "strcache.h"


//###############################################################
// strcache
//###############################################################
StrCache::StrCache(int cnt)
{
    this->cnt = cnt;
    access = 0;
    lines = new CacheLine[cnt];
    memset(lines, 0, sizeof(CacheLine) * cnt);

    DEBUG("StrCache %d\n", cnt);
}


int StrCache::get(int key)
{
    int i;
    for (i = 0; i < cnt; i++)
    {
        if (lines[i].key == key)
        {
            lines[i].access = ++access;
            return i;
        }
    }
    //DEBUG("MISS\n");
    return CACHE_MISS;
}


void StrCache::put(int key, char * buf, int flags)
{
    int oldest_index = 0;
    int oldest_time = 0;
    int i;
    for (i = 0; i < cnt; i++)
    {
        if (lines[i].key == CACHE_EMPTY)
        {
            oldest_index = i;
            break;
        }
        int time = access - lines[i].access;
        if (time > oldest_time)
        {
            oldest_time = time;
            oldest_index = i;
        }
    }
    CacheLine * line = &lines[oldest_index];
    line->key = key;
    line->access = ++access;
    line->flags = flags;
    memcpy(line->txt, buf, sizeof(line->txt));
}


