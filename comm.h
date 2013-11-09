#ifndef __COMM_H__
#define __COMM_H__

#include <public.h>

extern uint8_t commIsPaired;
extern uint8_t commIsLocked;
extern uint16_t mykey;

extern void showColor (uint16_t u);

void commInit ();
void commProcess ();

#endif
