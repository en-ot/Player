#ifndef _PAGE_FILES_H_
#define _PAGE_FILES_H_

#include "player.h"
#include "player_ctrl.h"


class PageFiles : public Page
{
public:
    PageFiles(Gui * gui);
    void box(int dircnt);
    void goto_cur();
    void go_to(int line);
    void activate();
    void update();

    class PageFilesPrivate * g;
};

//extern PageFiles page_files;


#endif // _PAGE_FILES_H_
