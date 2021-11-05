#include <Arduino.h>
#include "debug.h"

#include "playlist.h"

#include "SD_Libs.h"


//###############################################################
int clamp1(int num, int cnt)
{
    return (num + cnt - 1) % cnt + 1;
}


//###############################################################
typedef struct
{
    File dir;
    int first_file;
    int first_dir;
} Ent;


class PlaylistPrivate
{
public:
    void enter_safe();
    void leave_safe();
    File sd_open(String path);
    size_t entry_name(File & file, char * buf, int len);
    void rewind_directory(File & dir);
    File open_next_file(File & dir);
    void close(File & file);
    bool is_dir(File & file);

    Ent dirs[DIR_DEPTH];
    File entry;
    int thread;
};


//###############################################################
// wrapper
//###############################################################
extern TaskHandle_t audio_task_handle;

void PlaylistPrivate::enter_safe()
{
    if (thread != DIRECT_ACCESS) 
        vTaskSuspend(audio_task_handle);
}


void PlaylistPrivate::leave_safe()
{
    if (thread != DIRECT_ACCESS) 
        vTaskResume(audio_task_handle);
}


File PlaylistPrivate::sd_open(String path)
{
    enter_safe();
    File result = SD.open(path);
    leave_safe();
    return result;
}


size_t PlaylistPrivate::entry_name(File & file, char * buf, int len)
{
    buf[0] = 0;

    if (file)
    {
        enter_safe();
        size_t result = file.getName(buf, len);
        leave_safe();
        return result;
    }

    return 0;
}


void PlaylistPrivate::rewind_directory(File & dir)
{
    enter_safe();
    dir.rewindDirectory();
    leave_safe();
}


void PlaylistPrivate::close(File & dir)
{
    enter_safe();
    dir.close();
    leave_safe();
}


File PlaylistPrivate::open_next_file(File & dir)
{
    enter_safe();
    File result = dir.openNextFile();
    leave_safe();
    return result;
}


bool PlaylistPrivate::is_dir(File & file)
{
    enter_safe();
    bool result = file.isDirectory();
    leave_safe();
    return result;
}


//###############################################################
void Playlist::rewind()
{
    p->entry = p->sd_open(root_path);
    level = 0;
    curfile = 1;
    curdir = 1;
    p->dirs[level].dir = p->entry;
    p->dirs[level].first_dir = curdir;
    p->dirs[level].first_file = curfile;
}


void Playlist::rewind_to_file(int file_num)
{
//    Serial.printf("rewind %d\n", file_num);
    while (level > 0)
    {
//        Serial.printf("level=%d file=%d\n", level, p->dirs[level].first_file);
        if (p->dirs[level].first_file <= file_num)
        {
            curfile = p->dirs[level].first_file;
            curdir = p->dirs[level].first_dir;
            p->rewind_directory(p->dirs[level].dir);
            p->entry = p->dirs[level].dir;
            return;
        }
        p->close(p->dirs[level].dir);
        level -= 1;
    }

    rewind();
}


void Playlist::rewind_to_dir(int dir_num)
{
    while (level > 0)
    {
        if (p->dirs[level].first_dir <= dir_num)
        {
            curfile = p->dirs[level].first_file;
            curdir = p->dirs[level].first_dir;
            p->rewind_directory(p->dirs[level].dir);
            p->entry = p->dirs[level].dir;
            return;
        }
        p->close(p->dirs[level].dir);
        level -= 1;
    }

    rewind();
}


bool Playlist::find_file0(int file_num)
{
    while (true)
    {
        if (curfile == file_num)
        {
            return true;
        }

        p->entry = p->open_next_file(p->dirs[level].dir);
        if (!p->entry) // no more files
        {
            p->close(p->dirs[level].dir);
            if (level == 0)
                return false;
            level -= 1;
            continue;
        }

        curfile += 1;

        if (p->is_dir(p->entry))
        {
            curdir += 1;
            if (level+1 < DIR_DEPTH)   //skip deep dirs
            {
                level += 1;
                p->dirs[level].dir = p->entry;
                p->dirs[level].first_dir = curdir;
                p->dirs[level].first_file = curfile;
            }
        }
    }
}


//###############################################################
// Playlist Class
//###############################################################
Playlist::Playlist(int thread_id)
{
    root_path = "/";
    p = new PlaylistPrivate();
    p->thread = thread_id;
}


void Playlist::set_root(String path)
{
    root_path = path;
    uint32_t t0 = millis();
    scan_files(&filecnt, &dircnt);
    DEBUG("root: %s, dirs: %i, files: %i, time: %lu\n", root_path.c_str(), dircnt, filecnt, millis() - t0);
}


void Playlist::copy_from(Playlist * other)
{
    filecnt = other->filecnt;
    dircnt = other->dircnt;
    root_path = other->root_path;
    rewind();
}


//###############################################################
size_t Playlist::file_name(int file_num, char * dst, int len)
{
    if (!find_file(file_num))
    {
        dst[0] = 0;
        return 0;
    }

    return p->entry_name(p->entry, dst, len);
}


size_t Playlist::file_dirname(int file_num, char * dst, int len)
{
    if (!find_file(file_num))
    {
        dst[0] = 0;
        return 0;
    }

    root_path.toCharArray(dst, len);
    int x = strlen(dst);
    DEBUG("dirname: %s[%d]\n", dst, x);
    if (x == 1) 
        x = 0;
    len -= x;
    int i;
    for (i = 1; i <= level; i++)
    {
        dst[x++] = '/';
        len--;
        int len1 = p->entry_name(p->dirs[i].dir, &dst[x], len);
        x += len1;
        len -= len1;
        if (!len)
            break;
    }
    return x;
}


size_t Playlist::file_pathname(int file_num, char * dst, int len)
{
    int x;
    x = file_dirname(file_num, dst, len);
    if (!x)
        return 0;
    dst[x++] = '/';
    x += p->entry_name(p->entry, &dst[x], len-x);
    return x;
}


bool Playlist::file_is_dir(int file_num)
{
    if (!find_file(file_num))
        return false;

    return p->is_dir(p->entry);
}


//###############################################################
bool Playlist::find_file(int file_num)
{
    if (curfile > file_num)
    {
        rewind_to_file(file_num);
    }

    if (find_file0(file_num))
    {
        return true;
    }
    
    rewind();

    return false;
}


void Playlist::scan_files(int *fcnt, int *dcnt)
{
    rewind();
    find_file0(MAX_FILE_NUMBER);
    *fcnt = curfile;
    *dcnt = curdir;
}


bool Playlist::find_dir(int dir_num)
{
    dir_num = clamp1(dir_num, dircnt);

    //DEBUG("Find dir %d\n", dir_num);

    if (curdir >= dir_num)
    {
        //DEBUG("Rewind\n");
        rewind_to_dir(dir_num);
    }
    //DEBUG("Dir %d, File %d\n", curdir, curfile);

    int file_num = curfile;
    while (curdir != dir_num) 
    {
        bool res = find_file0(file_num);
        //DEBUG("Dir %d, File %d\n", curdir, curfile);
        if (!res)
        {
            DEBUG("Not found\n");
            return false;
        }
        file_num += 1;
    }

    //DEBUG("Dir %d found\n", curdir);
    return true;
}

