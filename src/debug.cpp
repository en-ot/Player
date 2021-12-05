#include <Arduino.h>
#include "debug.h"

uint32_t debug_val = 0;

void DEBUG_DUMP8(const void * addr, int len, int wdt)
{
    uint8_t *ptr = (uint8_t *)addr;
    int i;
    while (len)
    {
        DEBUG("%08X: ", (uint32_t)ptr);
        for (i = 0; i < wdt; i++)
        {
            DEBUG("%02X ", *ptr);
            ptr++;
            len--;
        }
        DEBUG("\n");
    }
}

void DEBUG_DUMP32(const void * addr, int len, int wdt)
{
    uint32_t *ptr = (uint32_t *)addr;
    int i;
    while (len)
    {
        DEBUG("%08X: ", (uint32_t)ptr);
        for (i = 0; i < wdt; i++)
        {
            DEBUG("%08X ", *ptr);
            ptr++;
            len--;
        }
        DEBUG("\n");
    }
}


void * xmalloc(size_t __size)
{
    void * p = malloc(__size);
    Serial.printf("malloc %08X[%d]\n", (unsigned int)p, __size);
    return p;
}


void * xcalloc(size_t __nmemb, size_t __size)
{
    void * p = calloc(__nmemb, __size);
    Serial.printf("calloc %08X[%dx%d]\n", (unsigned int)p, __nmemb, __size);
    return p;
}

