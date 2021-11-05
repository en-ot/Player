#ifndef _GUI_H_
#define _GUI_H_


#include <Arduino.h>
#include "elem/XListbox.h"

//#define XLISTBOX_MAX_STR 70  // длина одной строки экрана в байтах, не выделять память больше (UTF-8 - размер до 3x, 20 символов=60 байт)
//#define MAX_STR 70  // длина одной строки экрана в байтах, не выделять память больше (UTF-8 - размер до 3x, 20 символов=60 байт)
#define INFO_INDEX_MAX_STR 20
#define INFO_LINE_MAX_STR 250
#define LISTBOX_LINES 8

enum {PAGE_INFO, PAGE_FILES, /*PAGE_PIC,*/ PAGE_FAV, PAGE_DIRS, PAGE_SYS, PAGE_MAX};


class Gui
{
    private:
        bool initialized = false;
        // void mode();

    public:
        Gui();
        
        void loop();
        void set_page(int p);
        void redraw();
};


//extern Gui * gui;

void check_font();

#endif // _GUI_H_
