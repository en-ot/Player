#include <Arduino.h>
#include "debug.h"

void DEBUG_DUMP8(const void * addr, int len, int wdt)
{
    uint8_t *ptr = (uint8_t *)addr;
    int i;
    while (len)
    {
        DEBUG("%08X: ", (uint32_t)addr);
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
        DEBUG("%08X: ", (uint32_t)addr);
        for (i = 0; i < wdt; i++)
        {
            DEBUG("%08X ", *ptr);
            ptr++;
            len--;
        }
        DEBUG("\n");
    }
}
