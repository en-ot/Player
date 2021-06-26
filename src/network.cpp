#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "debug.h"
#include "sound.h"
#include "gui.h"
#include "globals.h"

#include "network.h"
#include "credentials.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;


void ota_onEnd()
{
    gui->message("End");
    gui->loop();
}


void ota_onStart() 
{
    Serial.print("Start");
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    type = "sketch";
    else // U_SPIFFS
    type = "filesystem";
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("updating " + type);
    
    sound_pause();
    gui->step_begin("OTA Upload");
    gui->message(type.c_str());
    gui->loop();
}


void ota_onProgress(unsigned int progress, unsigned int total) 
{
    int percent = progress / (total / 100);
    Serial.printf("Progress: %u%%\r", percent);
    
    gui->step_progress(percent, 100);
    gui->loop();
}
 

void ota_onError(ota_error_t error) 
{
    char buf[30];
    sprintf(buf, "OTA Upload Error %u", error);
    gui->step_begin(buf);

    if      (error == OTA_AUTH_ERROR)       gui->error("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)      gui->error("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)    gui->error("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)    gui->error("Receive Failed");
    else if (error == OTA_END_ERROR)        gui->error("End Failed");
    gui->loop();
}


void network_init()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    ArduinoOTA.onEnd(ota_onEnd);
    ArduinoOTA.onStart(ota_onStart);
    ArduinoOTA.onProgress(ota_onProgress);
    ArduinoOTA.onError(ota_onError);
}
    

bool network_connected = false;


bool network_address(char * buf, int len)
{
    IPAddress ip = WiFi.localIP();
    snprintf(buf, len, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}


void network_loop()
{
    if (!network_connected)
    {
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
            return;

        network_connected = true;
        DEBUG("IP address: ");
        Serial.print(WiFi.localIP());
        DEBUG("\n");

        ArduinoOTA.begin();
    }
    else
    {
        ArduinoOTA.handle();
    }
        // while (WiFi.waitForConnectResult() != WL_CONNECTED) 
    // {
    //     Serial.println("Connection Failed! Rebooting...");
    //     delay(5000);
    //     ESP.restart();
    // }

}

