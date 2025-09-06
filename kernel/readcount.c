// in kernel/readcount.c (create)
#include "types.h"
#include "defs.h"
#include "readcount.h"

uint64 global_read_bytes = 0;

void
add_read_bytes(unsigned int n)
{
  // natural wrap-around due to unsigned long long overflow
  global_read_bytes += (uint64)n;
}
