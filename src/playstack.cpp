#include <string.h>

#include "playstack.h"

//###############################################################
static int playstack[PLAYSTACK_LEVELS];
static int playstack_next_level = 0;


//###############################################################
void playstack_init()
{
    memset(playstack, PLAYSTACK_EMPTY, sizeof(playstack));
}


void playstack_push(int num)
{
    playstack[playstack_next_level] = num;
    playstack_next_level = (playstack_next_level + 1) % PLAYSTACK_LEVELS;
}


int playstack_pop()
{
    playstack_next_level = (playstack_next_level - 1 + PLAYSTACK_LEVELS) % PLAYSTACK_LEVELS;
    playstack[playstack_next_level] = PLAYSTACK_EMPTY;
    playstack_next_level = (playstack_next_level - 1 + PLAYSTACK_LEVELS) % PLAYSTACK_LEVELS;
    int n = playstack[playstack_next_level];
    playstack[playstack_next_level] = PLAYSTACK_EMPTY;
    return n;
}


int playstack_is_instack(int num)
{
    int i;
    for (i = 0; i < PLAYSTACK_LEVELS; i++)
    {
        int index = (playstack_next_level - i - 1 + PLAYSTACK_LEVELS) % PLAYSTACK_LEVELS;
        if (playstack[index] == num)
            return i;
    }
    return PLAYSTACK_NOT_IN_STACK;
}


//###############################################################
