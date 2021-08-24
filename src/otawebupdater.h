#ifndef _OTAWEBUPDATER_H_
#define _OTAWEBUPDATER_H_

#include <WebServer.h>
#include <Update.h>


extern WebServer server;
extern __attribute__((weak)) void otaweb_post2();

void otaweb_init();


#endif // _OTAWEBUPDATER_H_
