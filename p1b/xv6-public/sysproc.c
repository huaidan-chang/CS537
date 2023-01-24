#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
// #include "tracestat.h"
// int
// sys_trace(char *pathname)
// {
//   // if pathname is null return -1
//   if(pathname == 0){
//     return -1;
//   }

//   // if pathname is longer than 256 bytes
//   int count = 0;
//   while (count < 256 && *pathname) {
//     count++;
//     pathname++;
//   }
//   if (count >= 256 && *pathname) {
//     return -1;
//   }

//   cprintf("sys_trace: trace_enabled\n");
//   GLOBAL_STAT.trace_enabled = 1;
//   GLOBAL_STAT.trace_pathname = pathname;
//   GLOBAL_STAT.trace_count = 0;

//   return 0;
// } 

// int
// sys_getcount(void)
// {
//   cprintf("sys_getcount\n");
//   cprintf("GLOBAL_STAT.trace_count: %d \n", GLOBAL_STAT.trace_count);
//   return GLOBAL_STAT.trace_count;
// }

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
