// Settings for FTP client must be: one connection, no TLS


#include "Arduino.h"
#include "SPI.h"
#include "WiFi.h"
#include "WiFiClient.h"
#include "FS.h"
#include "SD.h"
#include "SD_MMC.h"
#include "ESP32FtpServer.h"

const char* ssid =     "SSID";       //WiFi SSID
const char* password = "PASSWD";     //WiFi Password


/*
 * Connect the SD_MMC card to the following pins:
 *
 * SD Card | ESP32  1bit mode
 *    CMD      15
 *    CLK      14
 *    CLK      14
 *    D0       2  (add 1K pull up after flashing)
 *
 * SD Card | ESP32  4bit mode
 *    D2       12
 *    D3       13
 *    CMD      15
 *    CLK      14
 *    D0       2  (add 1K pull up after flashing)
 *    D1       4
 */


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


/*
    // SD_MMC connection 1bit mode
    pinMode(2, INPUT_PULLUP); // hardware pullup might be necessary, see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/sd_pullup_requirements.html
    if (SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD opened in MMC mode (1 Bit)!");
        ftpSrv.begin(SD_MMC, "esp32","esp32");    //username, password for ftp.  set ports in ESP32FtpServer.h  (default 21, 50009 for PASV)
    }
*/

/*
    // start SD using the SD_MMC mode (4Bit mode)
    if (SD_MMC.begin()) {
        Serial.println("SD opened in MMC mode (4 Bit)!");
        ftpSrv.begin(SD_MMC, "esp32","esp32");    //username, password for ftp.  set ports in ESP32FtpServer.h  (default 21, 50009 for PASV)
    }
*/
}

void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}

// optional
void ftp_debug(const char* debug) {
    Serial.printf("ftpdebug: %s  \n", debug);
}
