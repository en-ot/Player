#include <stddef.h>

#define DEBUG(...) Serial.printf(__VA_ARGS__)

#define UNUSED(...) (void)__VA_ARGS__

extern uint32_t debug_val;

void DEBUG_DUMP8(const void * addr, int len, int wdt=16);
void DEBUG_DUMP32(const void * addr, int len, int wdt=4);

void * xmalloc(size_t __size);
void * xcalloc(size_t __nmemb, size_t __size);

