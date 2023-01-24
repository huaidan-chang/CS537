# Scheduling part

- Add a new `struct pstat` in `pstat.h`
- Add `tickets` and `tick` field to `struct proc` in `proc.h`
- Add a system call to set tickets of the calling process
    - `int settickets(int number)`
- Add a system call to retrieve all the running processes' information, including pid, inuse, tickets, and ticks, and store it in struct pstat.
    - `int getpinfo(struct pstat *)`
- Modify `allocproc()` in `proc.c` to initiate the value of tickets and ticks to 1 and 0
- Modify `fork()` in `proc.c` to let the child process have the same ticket as parent process
- Modify `scheduler()` in `proc.c` to ensure that processes with high priority are completed before running any processes with low priority, and that if several processes are in the same queue, they should run in round-robin mode.

# Virtual memory part

## Null-pointer Dereference

Because the user code is loaded at address 0 in xv6, a null-pointer dereference would not result in an exception. We should let the user code to be loaded into the second page, which is at address 0x1000, to make null-pointer dereference an exception.

To read the code from the second page, modify `exec()` in `exec.c` and `copyuvm()` in `vm.c`.

To allow the user code to be loaded into the second page, change the `-Ttext` argument value from `0` to `0x1000`.

## Read-only Code

Because the code's permissions in the xv6 are set to read-write rather than read-only, there may be some overwrite issues. I add two system calls to change the protection bit of some parts of the page table.

- `int mprotect(void *addr, int len)`
    - This system call makes the protection bits of the page range beginning at `addr` and of `len` pages to be read only.
- `int munprotect(void *addr, int len)`
    - This system call makes the protection bits of the page range beginning at `addr` and of `len` pages to be readable and writeable.

Both system calls would check that the len is greater than 0 and that the addr is aligned to the page and in the process's address space.