#include "network.h"

#ifdef NETWORK_ENABLED

#include <WiFi.h>
#include <ESPmDNS.h>

#include "credentials.h"

const char* host = "player";
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

const char* ap_ssid = "player";
const char* ap_password = "player_admin";
#define AP_IP 192,168,1,1

#endif

#include "debug.h"
#include "sound.h"
#include "gui.h"
#include "page_sys.h"
//#include "page_info.h"

#include "globals.h"




//###############################################################
#ifdef FTP_SERVER
#include "ESP32FtpServer.h"         // https://github.com/schreibfaul1/ESP32FTPServer

FtpServer ftpSrv;

void ftp_init()
{
    ftpSrv.begin(SD, "esp32", "esp32"); //username, password for ftp.
}


void ftp_begin()
{
}


void ftp_loop()
{
    ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}


bool writeflag = false;

void ftp_callback(int event, const char* text)
{
    Serial.printf("ftpdebug: %s\n", text);

    switch(event)
    {
        case FTPSERV_CLIENT_CONNECTED:
            player->freeze();
            writeflag = false;
        case FTPSERV_READY:
            sys.net(NET_MODE_FTP);
            break;

        case FTPSERV_RECEIVING:
            sys.net(NET_MODE_WRITE);
        case FTPSERV_REN:
        case FTPSERV_RMDIR:
        case FTPSERV_MKDIR:
            writeflag = true;
            break;

        case FTPSERV_SENDING:
        case FTPSERV_LIST:
            sys.net(NET_MODE_READ);
            break;

        case FTPSERV_ERROR:
            sys.net(NET_MODE_ERROR);
            break;

        case FTPSERV_CLIENT_DISCONNECTED:
        case FTPSERV_CLIENT_DISCONNECTING:
            if (writeflag)
            {
                player->restart();
                writeflag = false;
            }
            else
            {
                player->unfreeze();
            }
            sys.net(WiFi.getMode());
            break;
    }
}

#endif // FTP_SERVER


//###############################################################
#ifdef NETWORK_ENABLED
void ota_onEnd()
{
    sys.message("End");
    gui->loop();
    player->restart();
    sys.net(WiFi.getMode());
}


void ota_onStart(bool sketch) 
{
    player->freeze();
    sys.step_begin("OTA Upload ");
    const char * type = sketch ? "sketch" : "filesystem";
    Serial.println(type);
    sys.message(type);
    gui->loop();
    sys.net(NET_MODE_OTA);
}


void ota_onProgress(unsigned int progress, unsigned int total) 
{
    int percent = progress / (total / 100);
    Serial.printf("Progress: %u%%\r", percent);
    
    sys.step_progress(percent, 100);
    gui->loop();
}
 

void ota_onError(int error, const char * errtxt) 
{
    char buf[30];
    sprintf(buf, "OTA Upload Error %u", error);
    sys.step_begin(buf);
    sys.error(errtxt);
    gui->loop();
    player->restart();
}
#endif

//###############################################################
#ifdef HTTP_UPDATER

#include <WiFiClient.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>


WebServer httpServer(80);
HTTPUpdateServer httpUpdater;

void httpupdater_begin()
{
    httpUpdater.setup(&httpServer);
    httpServer.begin();
}

#endif // HTTP_UPDATER


//###############################################################
#ifdef OTA_WEB_UPDATER

#include "otawebupdater.h"


void otaweb_post2()
{
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) 
    {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        ota_onStart(true);
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))  //start with max available size
        {
            Update.printError(Serial);
            ota_onError(Update.getError(), "Error");
        }
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) // flashing firmware to ESP
    {
        ota_onProgress(Update.progress(), Update.size());
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) 
        {
            Update.printError(Serial);
            ota_onError(Update.getError(), "Error");
        }
    } 
    else if (upload.status == UPLOAD_FILE_END) 
    {
        if (Update.end(true)) //true to set the size to the current progress
        { 
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            ota_onEnd();
        } 
        else 
        {
            Update.printError(Serial);
            ota_onError(Update.getError(), "Error");
        }
    }
}


void otaweb_begin()
{
    server.begin();
}


void otaweb_loop()
{
    server.handleClient();
}

#endif // OTA_WEB_UPDATER


//###############################################################
#ifdef ARDUINO_OTA

#include <WiFiUdp.h>
#include <ArduinoOTA.h>


const char * arduino_ota_error(ota_error_t error) 
{
    switch(error)
    {
    case OTA_AUTH_ERROR):   return "Auth Failed";
    case OTA_BEGIN_ERROR:   return "Begin Failed";
    case OTA_CONNECT_ERROR: return "Connect Failed";
    case OTA_RECEIVE_ERROR: return "Receive Failed";
    case OTA_END_ERROR:     return "End Failed";    
    }
    return "?";
}


void arduinoota_init()
{
    ArduinoOTA.onStart([](){ota_onStart(ArduinoOTA.getCommand() == U_FLASH));
    ArduinoOTA.onProgress(ota_onProgress);
    ArduinoOTA.onError([](ota_error_t error) {ota_onError(error, arduino_ota_error(error)));
    ArduinoOTA.onEnd(ota_onEnd);
}

#endif // ARDUINO_OTA


//###############################################################
bool network_connected1 = false;


bool network_connected()
{
#ifdef NETWORK_ENABLED
    if (WiFi.getMode() == WIFI_AP)
        return true;
    return WiFi.isConnected() && network_connected1;
#else
    return false;
#endif
}


#ifdef NETWORK_ENABLED
IPAddress wifi_ip()
{
    IPAddress ip;
    switch(WiFi.getMode())
    {
    case WIFI_MODE_STA:
        ip = WiFi.localIP();
        break;
    
    case WIFI_MODE_AP:
        ip = WiFi.softAPIP();
        break;
    
    default:
        ip = INADDR_NONE;
    }

    return ip;
}


void wifi_ap()
{
    DEBUG("Connecting as AP\n");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(AP_IP), IPAddress(AP_IP), IPAddress(255,255,255,0));
    WiFi.softAP(ap_ssid, ap_password);
}


void wifi_sta()
{
    DEBUG("Connecting as STA\n");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}


void wifi_off()
{
    sys.net(WIFI_MODE_NULL);
    WiFi.disconnect();
    DEBUG("WiFi disconnected\n");
    network_connected1 = false;
}


void services_init()
{
#ifdef ARDUINO_OTA
    arduinoota_init();
#endif

#ifdef OTA_WEB_UPDATER
    otaweb_init();
#endif

#ifdef FTP_SERVER
    ftp_init();
#endif
}


void services_begin()
{
#ifdef ARDUINO_OTA
    ArduinoOTA.begin();
#endif

#ifdef HTTP_UPDATER
    httpupdater_begin();
#endif

#ifdef OTA_WEB_UPDATER
    otaweb_begin();
#endif

#ifdef FTP_SERVER
    ftp_begin();
#endif
}


void services_loop()
{
#ifdef ARDUINO_OTA
    ArduinoOTA.handle();
#endif

#ifdef HTTP_UPDATER
    httpServer.handleClient();
#endif

#ifdef OTA_WEB_UPDATER
    otaweb_loop();
#endif

#ifdef FTP_SERVER
    ftp_loop();
#endif
}


#endif
//###############################################################


void network_init()
{
#ifdef NETWORK_ENABLED
    wifi_ap();
    //wifi_sta();

    services_init();
#endif
}


uint32_t net_t0 = 0;

void network_reconnect(bool ap)
{
#ifdef NETWORK_ENABLED
    wifi_off();

    if (ap)
    {
        wifi_ap();
    }
    else
    {
        wifi_sta();
    }
    net_t0 = millis();
#endif
}


bool network_address(char * buf, int len)
{
#ifdef NETWORK_ENABLED
    IPAddress ip = wifi_ip();
    if (!ip)
        return false;
    
    snprintf(buf, len, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#endif
    return true;
}


//static uint8_t xxx[50000];

#define NET_TIMEOUT 10000

void network_loop()
{
#ifdef NETWORK_ENABLED
    if (network_connected1)
    {
        services_loop();
        return;
    }

    if (WiFi.getMode() == WIFI_MODE_STA)
    {
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            if ((int32_t)(millis() - net_t0) < NET_TIMEOUT)
                return;
            
            wifi_ap();
            net_t0 = millis();
            return;
        }
    }

    network_connected1 = true;
    DEBUG("WiFi Connected. IP address: ");
    Serial.print(wifi_ip());
    DEBUG("\n");

    MDNS.begin(host);
    services_begin();

    sys.net(WiFi.getMode());
    page_sys.update();
#else
    // xxx = (uint8_t*)malloc(100000);
    //xxx[0]++;
#endif

}

