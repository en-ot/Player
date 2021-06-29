#include <WiFi.h>
#include <ESPmDNS.h>

#include "debug.h"
#include "sound.h"
#include "gui.h"
//#include "globals.h"

#include "network.h"
#include "credentials.h"

const char* host = "player";
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;


//###############################################################
//#include "Sd_Libs.h"                  // https://github.com/greiman/SdFat
#include "ESP32FtpServer.h"         // https://github.com/schreibfaul1/ESP32FTPServer

FtpServer ftpSrv;

void ftp_init()
{
}


void ftp_begin()
{
    ftpSrv.begin(SD, "esp32", "esp32"); //username, password for ftp.
}


void ftp_debug(const char* debug) 
{
    Serial.printf("ftpdebug: %s  \n", debug);
}


void ftp_loop()
{
    ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}


//###############################################################
void ota_onEnd()
{
    gui->message("End");
    gui->loop();
}


void ota_onStart(bool sketch) 
{
    sound_pause();
    gui->step_begin("OTA Upload ");
    const char * type = sketch ? "sketch" : "filesystem";
    Serial.println(type);
    gui->message(type);
    gui->loop();
}


void ota_onProgress(unsigned int progress, unsigned int total) 
{
    int percent = progress / (total / 100);
    Serial.printf("Progress: %u%%\r", percent);
    
    gui->step_progress(percent, 100);
    gui->loop();
}
 

void ota_onError(int error, const char * errtxt) 
{
    char buf[30];
    sprintf(buf, "OTA Upload Error %u", error);
    gui->step_begin(buf);
    gui->error(errtxt);
    gui->loop();
}


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
    MDNS.addService("http", "tcp", 80);
}

#endif


//###############################################################
#ifdef OTA_WEB_UPDATER
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

// Login page
const char* loginIndex =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";


// Server Index Page
const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";


void otaweb_login()
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
}


void otaweb_index()
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
}


void otaweb_post1()
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}


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


void otaweb_init()
{
    server.on("/", HTTP_GET, otaweb_login);
    server.on("/serverIndex", HTTP_GET, otaweb_index);
    server.on("/update", HTTP_POST, otaweb_post1, otaweb_post2);
}


void otaweb_begin()
{
    server.begin();
}


void otaweb_loop()
{
    server.handleClient();
}

#endif


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

#endif


//###############################################################
void ota_init()
{
#ifdef ARDUINO_OTA
    arduinoota_init();
#endif

#ifdef OTA_WEB_UPDATER
    otaweb_init();
#endif
}


void ota_begin()
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
}


void ota_loop()
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
}


//###############################################################
bool network_connected = false;

void network_init()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    ota_init();
    ftp_init();
}


bool network_address(char * buf, int len)
{
    IPAddress ip = WiFi.localIP();
    snprintf(buf, len, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return true;
}


void network_loop()
{
    if (network_connected)
    {
        ota_loop();
        ftp_loop();
        return;
    }

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
        return;

    network_connected = true;
    DEBUG("IP address: ");
    Serial.print(WiFi.localIP());
    DEBUG("\n");

    MDNS.begin(host);
    ota_begin();
    ftp_begin();
}

