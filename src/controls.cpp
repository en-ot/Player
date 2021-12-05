#include <Arduino.h>
#include "debug.h"

#include "InputButton.h"
#include "AnalogEncoder.h"

#include "globals.h"

#include "pinout.h"
#include "player_input.h"

#include "controls.h"


InputButton btn_1(BTN_1, true, ACTIVE_LOW);
InputButton btn_2(BTN_2, true, ACTIVE_LOW);
InputButton btn_3(BTN_3, true, ACTIVE_LOW);

AnalogEncoder enc1(AENC1);
AnalogEncoder enc2(AENC2);

static bool (*_input)(PlayerInputType type, int key) = nullptr;


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
    if (!_input)
        return false;
        
    if (_input(I_SEEK1,  enc1.get_move())) return true;
    if (_input(I_SEEK2, -enc2.get_move())) return true;

    if (enc1.long_press())              return _input(I_BUTTON, BTN_VOLLONG);
    if (enc1.short_press())             return _input(I_BUTTON, BTN_VOLSHORT);
    
    if (enc2.long_press())              return _input(I_BUTTON, BTN_SEEKLONG);
    if (enc2.short_press())             return _input(I_BUTTON, BTN_SEEKSHORT);

    if (btn_1.longPress())              return _input(I_BUTTON, BTN_B1LONG);
    if (btn_1.shortPress())             return _input(I_BUTTON, BTN_B1SHORT);

    if (btn_2.longPress())              return _input(I_BUTTON, BTN_B2LONG);
    if (btn_2.shortPress())             return _input(I_BUTTON, BTN_B2SHORT);

    if (btn_3.longPress())              return _input(I_BUTTON, BTN_B3LONG);
    if (btn_3.shortPress())             return _input(I_BUTTON, BTN_B3SHORT);

    return false;
}


//###############################################################
void serial_loop()
{
    if (!Serial.available()) 
        return;

    char r;
    Serial.read(&r, 1);

    if (_input)
        _input(I_KEY, r);
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


//###############################################################
bool dummy_input(PlayerInputType type, int key)
{
    static int cnt = 0;
    if ((type == I_SEEK1 || type == I_SEEK2) && (key == 0))
        return false;
    cnt++;
    DEBUG("UNEXPECTED INPUT %d %d\n", type, key);
    debug_val = (cnt << 24) | (type << 16) | (key & 0xFFFF);
    return false;
}


void controls_init(bool (*callback)(PlayerInputType type, int key))
{
    Serial.flush();

    _input = dummy_input;
    controls_loop();    //flush input
    
    _input = callback;
    xTaskCreatePinnedToCore(enc_task, "enc_task", 5000, NULL, 2, &enc_task_handle, CONTROLS_CORE);
}


