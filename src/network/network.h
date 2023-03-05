void network_init();
void network_loop();

bool network_connected();
bool network_address(char * buf, int len);
void network_reconnect(int net_index);

#define NETWORK_ENABLED 1
//#define ARDUINO_OTA 1     // small but ugly
//#define HTTP_UPDATER 1
#define OTA_WEB_UPDATER 1
#define FTP_SERVER 1

#define WIFI_MODE_STA0      0
#define WIFI_MODE_HOME      1
#define WIFI_MODE_PHONE     2

#define NET_MODE_FTP 3
#define NET_MODE_OTA 4

#define NET_MODE_WRITE 5
#define NET_MODE_READ 6
#define NET_MODE_ERROR 7
