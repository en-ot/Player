#include <Arduino.h>

#include "AnalogEncoder.h"

#define DEBOUNCE_DELAY_MS 10
#define PRESS_DELAY_LONG_MS 1000

AnalogEncoder::AnalogEncoder(int pin)
{
    this->pin = pin;
    pinMode(pin, ANALOG);
    dir = 0;
    last_code = 4;
    pass = false;
    short_flag = false;
    long_flag = 0;
    cnt = 0;
    cnt_old = 0;
}

//            000 001   010   011   100   101   110  111
//            0, 1174, 1940, 2330, 2540, 2750, 2930, 3056
int aencv[] = {587, 1557, 2315, 2435, 2645, 2840, 2993};


void AnalogEncoder::process_button(bool new_state)
{
    unsigned int t = millis();
    if (button_raw != new_state)
    {
        button_raw = new_state;
        debounce_time = t;
        long_time = t;
        return;
    }

    if (t - debounce_time < DEBOUNCE_DELAY_MS)
        return;

    if (button_debounced != button_raw)
    {
        button_debounced = button_raw;
        if (button_debounced)
        {
            long_flag = 0;
        }
        else if (long_flag == 0)
        {
            short_flag = true;
        }
    }

    if ((t - long_time > PRESS_DELAY_LONG_MS) && button_debounced && (long_flag == 0))
    {
        long_flag = 1;
    }
}


void AnalogEncoder::process_position(int code)
{
    switch(code)
    {
    case 3:
        pass = true;
        break;
    case 1:
        dir = 1;
        break;
    case 2:
        dir = -1;
        break;
    case 0:
        if (!pass)
            return;
        portENTER_CRITICAL(&timer_mux);
        cnt += dir;
        portEXIT_CRITICAL(&timer_mux);
        pass = false;
        break;
    }
}


void AnalogEncoder::process()
{
    int x1 = analogRead(pin);
    int code = 3;
    code += (x1 > aencv[code]) ? 2 : -2;
    code += (x1 > aencv[code]) ? 1 : -1;
    code += (x1 > aencv[code]);
    
    if (last_code != code)
    {
        last_code = code;
        return;
    }
    last_code = code;

    process_button((code & 0x04) != 0);
    process_position(code & 0x03);
}


int AnalogEncoder::get_cnt()
{
    portENTER_CRITICAL(&timer_mux);
    int cnt1 = cnt;
    portEXIT_CRITICAL(&timer_mux);
    cnt_old = cnt1;
    return cnt1;
}


int AnalogEncoder::get_move()
{
    portENTER_CRITICAL(&timer_mux);
    int cnt1 = cnt;
    portEXIT_CRITICAL(&timer_mux);
    int delta = cnt1 - cnt_old;
    cnt_old = cnt1;
    return delta;
}


bool AnalogEncoder::is_pressed()
{
     return button_debounced;
}


bool AnalogEncoder::long_press()
{
    if (long_flag != 1)
        return false;

    short_flag = false;
    long_flag = 2;
    return true;
}


bool AnalogEncoder::short_press()
{
    if (!short_flag)
        return false;

    short_flag = false;
    long_flag = 2;
    return true;
}


bool AnalogEncoder::press_and_repeat()
{
    return false;
}

