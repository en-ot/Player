#ifndef _PLAYSTACK_H_
#define _PLAYSTACK_H_

#define PLAYSTACK_LEVELS 20
#define PLAYSTACK_NOT_IN_STACK -1

void playstack_init();
void playstack_push(int num);
int playstack_pop();
int playstack_is_instack(int num);

#endif // _PLAYSTACK_H_
