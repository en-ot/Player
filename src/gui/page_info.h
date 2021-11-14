#ifndef _PAGE_INFO_H_
#define _PAGE_INFO_H_

#include "player.h"
#include "player_ctrl.h"


class PageInfo : public Page
{
public:
    PageInfo(Gui * gui);

    void gui_loop();
    void update();
    void activate();

    void loop2();
    void set_queue(QueueHandle_t tag_queue);
    void clear();

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

    void message(const char * message);
    void error(const char * errtxt);
    void step_begin(const char * text);
    void step_progress(uint32_t pos, uint32_t total);

    void scroll();
    void scroll_reset();

private:
    class PageInfoPrivate * g;
};

//extern PageInfo * page_info;


#endif // _PAGE_INFO_H_
