# ESP32FTPServer
Simple FTP Server for Espressif ESP32
Based on the work from https://github.com/HenrikSte/ESP32FTPServer and https://github.com/MollySophia/ESP32_FTPServer_SD (which again is based on https://github.com/robo8080/ESP32_FTPServer_SD) 

Just resized the global buffer and introduced method isConnected().<br />
SD_MMC is now supported (thanks to tueddy for the contribution).<br />
Be advised that only one simultaneous connection is possible. I recommend [Filezilla](https://filezilla-project.org/) as it's a multi-OS-platform and this parameter can be configured there.

```c++

#include "Arduino.h"
#include "SPI.h"
#include "WiFi.h"
#include "WiFiClient.h"
#include "FS.h"
#include "SD.h"
#include "ESP32FtpServer.h"

const char* ssid =     "SSID";       //WiFi SSID
const char* password = "PASSWD";     //WiFi Password

// defines for SD card SPI
#define SPI_MISO    19
#define SPI_MOSI    23
#define SPI_SCK     18

#define SD_CS       5


FtpServer ftpSrv;


void setup(void){
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {delay(500); Serial.print(".");}
    Serial.printf("\n Connected to %s, IP address: %s  ", ssid, WiFi.localIP().toString().c_str());


    // SD Card via SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    if (SD.begin(SD_CS)) ftpSrv.begin(SD, "esp32", "esp32"); //username, password for ftp.
}

void loop(void){
    ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}

// optional
void ftp_debug(const char* debug) {
    Serial.printf("ftpdebug: %s  \n", debug);
}

```
Breadboard
![Breadboard](https://github.com/schreibfaul1/ESP32FTPServer/blob/master/examples/FTP_Server.jpg)
