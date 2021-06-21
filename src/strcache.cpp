#include "strcache.h"

//###############################################################
// strcache
//###############################################################
StrCache::StrCache(int cnt, CacheLine * lines)
{
    access = 0;
    memset(lines, 0, sizeof(CacheLine) * cnt);
    this->lines = lines;
    this->cnt = cnt;
}


int cache_get_item(StrCache * cache, int key)
{
    int i;
    for (i = 0; i < cache->cnt; i++)
    {
        if (cache->lines[i].key == key)
        {
            cache->lines[i].access = ++cache->access;
            return i;
        }
    }
    return CACHE_MISS;
}


void cache_put_item(StrCache * cache, int key, char * buf, int flags)
{
    int oldest_index = 0;
    int oldest_time = 0;
    int i;
    for (i = 0; i < cache->cnt; i++)
    {
        if (cache->lines[i].key == CACHE_EMPTY)
        {
            oldest_index = i;
            break;
        }
        int time = cache->access - cache->lines[i].access;
        if (time > oldest_time)
        {
            oldest_time = time;
            oldest_index = i;
        }
    }
    CacheLine * line = &cache->lines[oldest_index];
    line->key = key;
    line->access = ++cache->access;
    line->flags = flags;
    memcpy(line->txt, buf, sizeof(line->txt));
}


void cache_init(StrCache * cache)
{
}

