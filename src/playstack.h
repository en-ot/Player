#ifndef _PLAYSTACK_H_
#define _PLAYSTACK_H_

#define PLAYSTACK_LEVELS 100    //~8 hours
#define PLAYSTACK_NOT_IN_STACK -1
#define PLAYSTACK_EMPTY 0

void playstack_init();
void playstack_push(int num);
int playstack_pop();
int playstack_is_instack(int num);

#endif // _PLAYSTACK_H_
