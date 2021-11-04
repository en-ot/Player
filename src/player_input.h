#ifndef _PLAYER_INPUT_H_
#define _PLAYER_INPUT_H_


typedef enum
{
    BTN_VOLLONG,          BTN_VOLSHORT,
    BTN_SEEKLONG,         BTN_SEEKSHORT, 
    BTN_B1LONG,           BTN_B1SHORT, 
    BTN_B2LONG,           BTN_B2SHORT, 
    BTN_B3LONG,           BTN_B3SHORT,
} PlayerButton;

typedef enum
{
    I_BUTTON,
    I_KEY,
    I_SEEK1, 
    I_SEEK2,
} PlayerInputType;

bool input(PlayerInputType type, int key);


#endif // _PLAYER_INPUT_H_
