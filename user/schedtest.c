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
    for(int i = 0; i < 2000000; i++) x += i;
  }
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf("Usage: schedtest nchildren maxwork\n");
    exit(1);
  }

  int sum_wait = 0;
  int sum_runtime = 0;
  int sum_turnaround = 0;

  int n = atoi(argv[1]);
  int maxwork = atoi(argv[2]);
  if(n > MAXCHILD) n = MAXCHILD;

  int start_ticks = uptime();

  // Spawn children
  for(int i = 0; i < n; i++){
    int delay = randnum(30);          // arrival offset in ticks
    int work = randnum(maxwork) + 1;  // random work [1..maxwork]

    int pid = fork();
    if(pid == 0){
      // CHILD
      delay_ticks(delay);

      // Burn CPU
      for(volatile int j = 0; j < work * 10000000; j++);

      exit(0);   // don't fetch times here!
    }
  }

  // Parent: collect stats with waitx
  for(int i = 0; i < n; i++){
    int wtime, rtime, tatime;
    int pid = waitx(&wtime, &rtime, &tatime);
    if(pid > 0){
      printf("CHILD %d: wait=%d runtime=%d turnaround=%d\n",
             pid, wtime, rtime, tatime);

             sum_runtime += rtime;
             sum_wait += wtime;
             sum_turnaround += tatime;
    } else {
      printf("waitx failed\n");
    }
  }

  int end_ticks = uptime();
  printf("Parent: all children finished in %d ticks\n", end_ticks - start_ticks);
  printf("=============================================STATISTICS===============================\n");
  printf("Average: wait=%d runtime=%d turnaround=%d\n",
         sum_wait / n, sum_runtime / n, sum_turnaround / n);
  exit(0);
}
