#ifndef _MY_DISPLAY_H_
#define _MY_DISPLAY_H_


#include <Arduino.h>
#include "elem/XListbox.h"

#define XLISTBOX_MAX_STR 70  // длина одной строки экрана в байтах, не выделять память больше (UTF-8 - размер до 3x, 20 символов=60 байт)
#define MAX_STR 70  // длина одной строки экрана в байтах, не выделять память больше (UTF-8 - размер до 3x, 20 символов=60 байт)

enum {PAGE_INFO, PAGE_LIST, /*PAGE_PIC,*/ PAGE_FAV, PAGE_DIRS, PAGE_MAX};


class Gui
{
    private:
        bool initialized = false;
        void mode();

    public:
        Gui();
        
        void loop();
        void page(int p);
        void redraw();

        // page:info
        void index(const char * text);
        void time_progress(uint32_t index, uint32_t total);
        void step_progress(uint32_t pos, uint32_t total);
        void band(const char * text);
        void artist(const char * text);
        void album(const char * text);
        void title(const char * text);
        void file(const char * text);
        void path(const char * text);
        void volume(int volume);
        void shuffle(bool val);
        void repeat(bool val);
        void alive(bool running);

        // page:list
        int list_selfile = 1;
        void list_seek(int by);
        void list_select(int curfile);
        void list_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb);
        void list_highlight(void *m_gui, void *pElemRef, int type);

        // page:fav
        int fav_selfile = 1;
        void fav_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb);
        void fav_seek(int by);
        void fav_set(int num);
        void fav_highlight(void *m_gui, void *pElemRef, int type);
        
        // page:dirs
        int dirs_seldir = 1;
        void dirs_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb);
        void dirs_seek(int by);
};


#endif // _MY_DISPLAY_H_
