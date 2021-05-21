#ifndef _ANALOG_ENCODER_H_
#define _ANALOG_ENCODER_H_

#include <Arduino.h>


class AnalogEncoder
{
    private:
        int pin;

        unsigned long debounce_time;    // last raw change time
        unsigned long long_time;        // last debounced change time;
        int button_raw;                 // raw value
        int button_debounced;           // debounced value
        bool short_flag;                // short pressed flag
        int long_flag;                 // long pressed flag
        void process_button(bool new_state);

        int dir;                // direction store
        int last_code;          // previus code for filtering
        bool pass;              // zero-pass-detect
        int cnt;                // current position
        int cnt_old;            // previous cnt;
        void process_position(int code);

        portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;

    public:
        AnalogEncoder(int pin);         // init

        void process();                 // process encoder
        int get_cnt();                  // get count value
        int get_move();                 // get count difference
        bool is_pressed();              // button is now pressed
        bool long_press();              // released after long press
        bool short_press();             // released after short press
        bool press_and_repeat();        // repeating

};


#endif // _ANALOG_ENCODER_H_
