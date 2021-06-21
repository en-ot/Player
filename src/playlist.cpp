#include <Arduino.h>
#include "debug.h"

#include "playlist.h"


//int curroot = 0;
//int rootcnt = 0;


//###############################################################
int clamp1(int num, int cnt)
{
    return (num + cnt - 1) % cnt + 1;
}


//###############################################################
// FileControl Class
//###############################################################
playlist::playlist()
{
    root_path = "/";
}


void playlist::set_root(String path)
{
    root_path = path;
    unsigned long t0 = millis();
    scan_files(&filecnt, &dircnt);
    DEBUG("root: %s, dirs: %i, files: %i, time: %lu\n", root_path.c_str(), dircnt, filecnt, millis() - t0);
}


//###############################################################


// bool FileControl::scan_roots(int root_num)
// {
//     root_path = "/";
//     File root = SD.open(root_path);
//     curroot = 1;

//     while (true)
//     {
//         if (curroot == root_num)
//             return true;

//         File file = root.openNextFile();
//         if (!file)
//             break;

//         if (file.isDirectory())
//         {
//             char fname[FILENAME_MAX_LEN];
//             filename(file, fname, sizeof(fname));
//             root_path = String(fname);
//             curroot += 1;
//         }
//     }

//     DEBUG("rootcnt: %i\n", curroot);
//     return false;
// }

// void FileControl::find_root(int root_num)
// {
//     root_num = clamp1(root_num, rootcnt);

//     filecnt = 0;
//     dircnt = 0;
//     scan_roots(root_num);

//     unsigned long t0 = millis();
//     scan_files(&filecnt, &dircnt);
//     DEBUG("root: %s, dirs: %i, files: %i, time: %lu\n", root_path.c_str(), dircnt, filecnt, millis() - t0);
// }


//###############################################################
size_t playlist::entry_name(File & file, char * buf, int len)
{
    buf[0] = 0;

    if (file)
        return file.getName(buf, len);

    return 0;
}


//###############################################################
size_t playlist::file_name(int file_num, char * dst, int len)
{
    if (!find_file(file_num))
    {
        dst[0] = 0;
        return 0;
    }

    return entry_name(entry, dst, len);
}


size_t playlist::file_dirname(int file_num, char * dst, int len)
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
        int len1 = entry_name(dirs[i], &dst[x], len);
        x += len1;
        len -= len1;
        if (!len)
            break;
    }
    return x;
}


size_t playlist::file_pathname(int file_num, char * dst, int len)
{
    int x;
    x = file_dirname(file_num, dst, len);
    if (!x)
        return 0;
    dst[x++] = '/';
    x += entry_name(entry, &dst[x], len-x);
    return x;
}


bool playlist::file_is_dir(int file_num)
{
    if (!find_file(file_num))
        return false;

    return entry.isDirectory();
}


//###############################################################
void playlist::rewind()
{
    entry = SD.open(root_path);
    level = 0;
    curfile = 1;
    curdir = 1;
    dirs[level] = entry;
}


bool playlist::find_file0(int file_num)
{
    while (true)
    {
        if (curfile == file_num)
        {
            return true;
        }

        entry = dirs[level].openNextFile();
        if (!entry) // no more files
        {
            dirs[level].close();
            if (level == 0)
                return false;
            level -= 1;
            continue;
        }

        curfile += 1;

        if (entry.isDirectory())
        {
            curdir += 1;
            if (level+1 < DIR_DEPTH)   //skip deep dirs
            {
                level += 1;
                dirs[level] = entry;
            }
        }

        if (curfile == file_num)
        {
            return true;
        }
    }
}


bool playlist::find_file(int file_num)
{
    if (curfile > file_num)
    {
        rewind();
    }

    if (find_file0(file_num))
    {
        return true;
    }
    
    rewind();

    return false;
}


void playlist::scan_files(int *fcnt, int *dcnt)
{
    rewind();
    find_file0(MAX_FILE_NUMBER);
    *fcnt = curfile;
    *dcnt = curdir;
}


bool playlist::find_dir(int dir_num)
{
    dir_num = clamp1(dir_num, dircnt);

    DEBUG("Find dir %d\n", dir_num);

    if (curdir >= dir_num)
    {
        rewind();
    }

    int file_num = curfile;
    while (curdir != dir_num) 
    {
        if (!find_file0(file_num))
        {
            DEBUG("Not found\n");
            return false;
        }
        file_num += 1;
    }

    return true;
}

