#include <Arduino.h>

#include "AnalogEncoder.h"

#define SERIAL_DEBUG

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
//int aenct[] = {587, 1557, 2315, 2435, 2645, 2840, 2993};
//int aencv[] = {0, 1174, 1940, 2330, 2540, 2750, 2930, 3056};


void AnalogEncoder::process_button(bool new_state)
{
    uint32_t t = millis();
    if (button_raw != new_state)
    {
        button_raw = new_state;
        debounce_time = t;
        long_time = t;
        return;
    }

    if ((int32_t)(t - debounce_time) < AE_DEBOUNCE_DELAY_MS)
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

    if (((int32_t)(t - long_time) > AE_PRESS_DELAY_LONG_MS) && button_debounced && (long_flag == 0))
    {
        long_flag = 1;
        //Serial.print(".");
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
    if (calibrate_mode)
        return;

    int x1 = analogRead(pin);
    int d1 = 5000;
    int code = 0;
    for (int i = 0; i < AE_ADC_STEPS; i++)
    {
        int delta = abs(aencv[i] - x1);
        if (delta < d1)
        {
            d1 = delta;
            code = i;
        }
    }

    // int code = 3;
    // code += (x1 > aenct[code]) ? 2 : -2;
    // code += (x1 > aenct[code]) ? 1 : -1;
    // code += (x1 > aenct[code]);
    
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


void AnalogEncoder::calibrate()
{
    calibrate_mode = true;

#ifdef SERIAL_DEBUG
    Serial.printf("calibrate pin %d start...\n", pin);
#endif

    uint16_t * calibrate_array = (uint16_t *)calloc(sizeof(uint16_t), AE_ADC_RANGE);
    for (int i = 0; i < 5000; i++)
    {
        uint16_t x1 = analogRead(pin);
        calibrate_array[x1]++;
        delay(1);

        if (!(i % 500) && ae_calibrate_callback)
            ae_calibrate_callback(i*100/500);
    }

#ifdef SERIAL_DEBUG
    Serial.printf("calibrate results:\n");
#endif

    for (int j = 0; j < AE_ADC_STEPS; j++)
    {
        int max_index = -1;
        for (int i = 0; i < AE_ADC_RANGE; i++)
        {
            if (calibrate_array[i] > calibrate_array[max_index])
            {
                max_index = i;
            }
        }
 
        if (max_index >= 0)
        {
            uint16_t max = calibrate_array[max_index];
            int sum = 0;
            for (int i = max_index-AE_ADC_HALF_WIDTH; i <= max_index+AE_ADC_HALF_WIDTH; i++)
            {
                if ((i >= 0) && (i < AE_ADC_RANGE))
                {
                    sum += calibrate_array[i];
                    calibrate_array[i] = 0;
                }
            }
#ifdef SERIAL_DEBUG
            Serial.printf("max_index %d value %d/%d\n", max_index, max, sum);
#endif
            aencv[j] = max_index;
        }
    }
    
    qsort(aencv, AE_ADC_STEPS, sizeof(uint16_t), [](const void *cmp1, const void *cmp2){return *((uint16_t *)cmp1) - *((uint16_t *)cmp2);});
#ifdef SERIAL_DEBUG
    for (int i = 0; i < AE_ADC_STEPS; i++)
    {
        Serial.printf("%d, ", aencv[i]);
    }
    Serial.printf("\n");
#endif

    if (ae_calibrate_callback)
        ae_calibrate_callback(100);

    free(calibrate_array);
    delay(1000);

    calibrate_mode = false;
}
