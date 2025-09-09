#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXCHILD 16

// Simple linear congruential generator for pseudo-random numbers
static unsigned randstate = 1;
int randnum(int max) {
  randstate = randstate * 1103515245 + 12345;
  return (randstate >> 16) % max;
}

// Delay for a number of ticks
void delay_ticks(int ticks) {
  int start = uptime();
  while(uptime() - start < ticks) {
    volatile int x = 0;
    for(int i = 0; i < 2000000000; i++) x += i;  // heavier busy work
  }
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf("Usage: schedtest nchildren maxwork\n");
    exit(1);
  }

  int n = atoi(argv[1]);
  int maxwork = atoi(argv[2]);
  if(n > MAXCHILD) n = MAXCHILD;

  int start_ticks = uptime();

  for(int i = 0; i < n; i++){
    int delay = randnum(30);          // arrival offset in ticks
    int work = randnum(maxwork) + 1;  // random work [1..maxwork]

    int pid = fork();
    if(pid == 0){
      // CHILD
      delay_ticks(delay);              // staggered arrival
      int submit = uptime();
      int s = uptime();

      // Burn CPU long enough to span multiple ticks
      for(volatile int j = 0; j < work * 10000000; j++);  // heavier CPU work

      int e = uptime();
      printf("CHILD %d: work=%d submit=%d start=%d end=%d runtime=%d wait=%d turnaround=%d\n",
             getpid(), work, submit, s, e, e-s, s-submit, e-submit);
      exit(0);
    }
  }

  for(int i = 0; i < n; i++)
    wait(0);

  int end_ticks = uptime();
  printf("Parent: all children finished in %d ticks\n", end_ticks - start_ticks);
  exit(0);
}
