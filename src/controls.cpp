#include <Arduino.h>
#include "debug.h"

#include "InputButton.h"
#include "AnalogEncoder.h"

#include "pinout.h"
#include "player.h"

#include "controls.h"


InputButton btn_1(BTN_1, true, ACTIVE_LOW);
InputButton btn_2(BTN_2, true, ACTIVE_LOW);
InputButton btn_3(BTN_3, true, ACTIVE_LOW);

AnalogEncoder enc1(AENC1);
AnalogEncoder enc2(AENC2);


//###############################################################
// Input controls
//###############################################################
TaskHandle_t enc_task_handle;
static void enc_task(void * pvParameters)
{
    while (true)
    {
        enc1.process();
        enc2.process();
        vTaskDelay(1);
    }
}


void controls_init()
{
    xTaskCreatePinnedToCore(enc_task, "enc_task", 5000, NULL, 2, &enc_task_handle, 0);
}


bool controls_defaults()
{
    return btn_1.isPressed();
}


void controls_pause()
{
    vTaskSuspend(enc_task_handle);
}


void controls_resume()
{
    vTaskResume(enc_task_handle);
}


bool input_loop()
{
    if (input(I_SEEK1,  enc1.get_move())) return true;
    if (input(I_SEEK2, -enc2.get_move())) return true;

    if (enc1.long_press())              return input(I_BUTTON, KEY_VOLLONG);
    if (enc1.short_press())             return input(I_BUTTON, KEY_VOLSHORT);
    
    if (enc2.long_press())              return input(I_BUTTON, KEY_SEEKLONG);
    if (enc2.short_press())             return input(I_BUTTON, KEY_SEEKSHORT);

    if (btn_1.longPress())              return input(I_BUTTON, KEY_B1LONG);
    if (btn_1.shortPress())             return input(I_BUTTON, KEY_B1SHORT);

    if (btn_2.longPress())              return input(I_BUTTON, KEY_B2LONG);
    if (btn_2.shortPress())             return input(I_BUTTON, KEY_B2SHORT);

    if (btn_3.longPress())              return input(I_BUTTON, KEY_B3LONG);
    if (btn_3.shortPress())             return input(I_BUTTON, KEY_B3SHORT);

    return false;
}


//###############################################################
void serial_loop()
{
    if (!Serial.available()) 
        return;

    char r;
    Serial.read(&r, 1);

    input(I_KEY, r);
}


void controls_loop()
{
    input_loop();
    serial_loop();
}


void controls_calibrate(int n)
{
    if (n == 1) enc1.calibrate();
    else enc2.calibrate();
}


void controls_set_prefs(uint8_t * src)
{
    memcpy(enc1.aencv, src, sizeof(enc1.aencv));
    src += sizeof(enc1.aencv);
    memcpy(enc2.aencv, src, sizeof(enc2.aencv));
}


void controls_get_prefs(uint8_t * dst)
{
    memcpy(dst, enc1.aencv, sizeof(enc1.aencv));
    dst += sizeof(enc1.aencv);
    memcpy(dst, enc2.aencv, sizeof(enc2.aencv));
}
