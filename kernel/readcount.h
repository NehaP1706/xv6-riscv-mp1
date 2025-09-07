// in kernel/readcount.h (create)
#ifndef READCOUNT_H
#define READCOUNT_H

#include "types.h"

extern uint64 global_read_bytes; // wrap at overflow (natural for unsigned)
void add_read_bytes(unsigned int n); // called by sys_read wrapper

#endif
//here
