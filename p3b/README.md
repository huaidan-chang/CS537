# Concurrency-xv6-threads
The goal of this project is to add true kernel threads to xv6. On the basis of the new system calls clone() and join(), a thread library including thread call(), thread join(), lock init(), lock acquire(), and lock release() is built.

int clone(void(*fcn)(void *, void *), void *arg1, void *arg2, void *stack) - This call creates a new kernel thread which shares the calling process's address space.

int join(void **stack) - This call waits for a child thread that shares the address space with the calling process to exit.

## Changes
### proc.c
1. Two new system calls clone and join are implemented.
2. wait() is modified to wait the child threads which share different address space and free the address space if this child thread is last reference to it.
3. growproc() is modified to update address space in threads if the parents' address space is updated.

### proc.h
Two new fields tstack and tcount are added to struct proc.

### defs.h
The declarations for clone and join are added.

### syscall.c
The declarations for sys_clone and sys_join are added.

### syscall.h
System call numbers for sys_clone and sys_join are added.

### sysproc.c
Two system calls, sys_clone and sys_join, are added, and these two functions call the clone and join implementations in proc.c.

### user.h
Two new system calls are defined, as well as the functions of the new thread library.

### usys.S
Two new system calls are added here for users to use in user mode.

### ulib.c
The new thread library functions including thread_create(), thread_join(), lock_init(), lock_acquire(), and lock_release() are added.

### Makefile
Appending umalloc.o to the first line of the _forktest rule to avoid "undefined reference to malloc ulib.c" error.
```
$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o _forktest forktest.o ulib.o usys.o umalloc.o
```