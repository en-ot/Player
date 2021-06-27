#define DEBUG(...) Serial.printf(__VA_ARGS__)

#define UNUSED(...) (void)__VA_ARGS__

void DEBUG_DUMP8(const void * addr, int len, int wdt=16);
void DEBUG_DUMP32(const void * addr, int len, int wdt=4);


