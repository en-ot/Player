void network_init();
void network_loop();

extern bool network_connected;
bool network_address(char * buf, int len);

//#define ARDUINO_OTA 1
