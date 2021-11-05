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
        void page(int p);
        void redraw();
        
        void scroll();
        void scroll_reset();

        void message(const char * message);
        void error(const char * errtxt);
        void step_begin(const char * text);
        void step_progress(uint32_t pos, uint32_t total);

        // page:info
        void index(const char * text);
        void fav(int fav_num);
        void time_progress(uint32_t index, uint32_t total);
        void band(const char * text);
        void artist(const char * text);
        void album(const char * text);
        void title(const char * text);
        void file(const char * text);
        void path(const char * text, const char * root);
        void volume(int volume);
        void gain(bool gain);
        void shuffle(bool val);
        void repeat(bool val);
        void alive(bool running);
        void net(int mode);
        void bluetooth(bool enabled);
};


extern gslc_tsGui gslc;

extern Gui * gui;
//extern int ui_page;

void check_font();

#endif // _GUI_H_
