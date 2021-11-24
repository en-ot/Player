#ifndef _ANALOG_ENCODER_H_
#define _ANALOG_ENCODER_H_

#include <Arduino.h>

#define AE_ADC_STEPS 8
#define AE_ADC_RANGE 4096
#define AE_ADC_HALF_WIDTH 10

#define AE_DEBOUNCE_DELAY_MS 10
#define AE_PRESS_DELAY_LONG_MS 500

extern __attribute__((weak)) void ae_calibrate_callback(int step);


class EncoderPosition
{
public:
    int codes = 0;            // previous codes store
    int cnt = 0;            // current position
    int cnt_old = 0;        // previous cnt
};


class AnalogEncoder
{
public:
    AnalogEncoder(int pin);         // init

    void process();                 // process encoder
    int get_cnt();                  // get count value
    int get_move();                 // get count difference
    bool is_pressed();              // button is now pressed
    bool long_press();              // released after long press
    bool short_press();             // released after short press
    bool press_and_repeat();        // repeating

    void calibrate();
    uint16_t aencv[AE_ADC_STEPS] = {0, 1174, 1940, 2330, 2540, 2750, 2930, 3056};

private:
    int pin;

    int last_code = -1;         // previus code for filtering

    uint32_t debounce_time;    // last raw change time
    uint32_t long_time;        // last debounced change time;
    int button_raw;                 // raw value
    int button_debounced;           // debounced value
    bool short_flag;                // short pressed flag
    int long_flag;                 // long pressed flag
    void process_button(bool new_state);

    EncoderPosition pos;
    void process_position(EncoderPosition & pos, int code);

    portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;

    bool calibrate_mode = false;
};


#endif // _ANALOG_ENCODER_H_
