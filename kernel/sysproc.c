#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "readcount.h"

extern struct proc proc[NPROC];

uint64
sys_waitx(void)
{
  uint64 up_w, up_r, up_t;
  argaddr(0, &up_w);
  argaddr(1, &up_r);
  argaddr(2, &up_t);
  return waitx(up_w, up_r, up_t);
}

uint64
sys_getprocesstimes(void)
{
  int pid;
  uint64 user_addr;
  struct proc *p;

  argint(0, &pid);
  argaddr(1, &user_addr);

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      struct times t;
      t.creation_time = p->creation_time;
      t.start_time    = p->start_time;
      t.end_time      = p->end_time;
      release(&p->lock);

      if(copyout(myproc()->pagetable, user_addr, (char*)&t, sizeof(t)) < 0)
        return -1;
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

uint64
sys_getreadcount(void)
{
  return global_read_bytes;
}

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
