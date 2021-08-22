void network_init();
void network_loop();

bool network_connected();
bool network_address(char * buf, int len);
void network_reconnect(bool ap);

#define NETWORK_ENABLED 1
//#define ARDUINO_OTA 1
//#define HTTP_UPDATER 1
//#define OTA_WEB_UPDATER 1
//#define FTP_SERVER 1

#define NET_MODE_FTP 3
#define NET_MODE_OTA 4
