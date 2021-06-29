// Settings for FTP client must be: one connection and no TLS. charset UTF8

// supports SdFat environment

#include "Arduino.h"
#include "SPI.h"
#include "WiFi.h"
#include "WiFiClient.h"
#include "SdFat.h"                  // https://github.com/greiman/SdFat
#include "ESP32FtpServer.h"         // https://github.com/schreibfaul1/ESP32FTPServer

const char* ssid =     "xxxxxxxx";  //WiFi SSID
const char* password = "xxxxxxxx";  //WiFi Password

// defines for SD SPI
#define SPI_MISO    19
#define SPI_MOSI    23
#define SPI_SCK     18

#define SD_CS       5

FtpServer ftpSrv;


void setup(void){
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {delay(500); Serial.print(".");}
    Serial.printf("\nConnected to %s, IP address: %s\n", ssid, WiFi.localIP().toString().c_str());

    // SD Card via SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    if (SD.begin(SD_CS)){
        ftpSrv.begin(SD, "esp32", "esp32"); //username, password for ftp.
    }
}

void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}

// optional
void ftp_debug(const char* debug) {
    Serial.printf("ftpdebug: %s  \n", debug);
}

