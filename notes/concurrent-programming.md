# Chapter 12 Concurrent Programming

## Processes

- File tables are shared.

- User address space are not.
  - One process cannot overwrite the virtual memory of another process.
  - It is more difficult for processes to share state information.

- The overhead for process control and IPC is high.


## I/O Multiplexing

Use *select* function to ask the kernel to suspend the process until one or more I/O events have occurred.

```C
#include <sys/select.h>

int select(int n, fd_set *fdset, NULL, NULL, NULL);
/*
fdset: read set.
n: the cardinality of the read set.
Returns: nonzero count of the ready descriptors, -1 on error.
select blocks until at least one descriptor in the read set is ready for reading.
*/
```

**Pros**

- Event-driven designs give programmers more control over the behavior of their programs than process-based designs.

- Every logical flow has access to the entire address space of the process.

- Easy to debug like any sequential program.

- More efficient for not require a process context switch.

**Cons**

- Coding complexity.

- It makes event-driven server vulnerable to malicious client. 

- It cannot fully utilize multi-core processors.

## Threads

### pthread

- *pthread_create*

- *pthread_join*
  reap terminated thread

- *pthread_self*
  thread ID

- *pthread_cancel*
  terminate a thread with it's ID

- *pthread_detach*
  A joinable thread can be reaped and killed by other threads. Its memory resources
  are not freed until it is reaped by others. A detached thread cannot be reaped
  or killed by others. Its memory resources are freed automatically.

### Threads Memory Model

*Thread context*: 
- a thread ID
- stack
- stack pointer
- program counter
- condition codes
- general-purpose register values

Each thread shares the rest of the process context with the other threads.

### Synchronizing Threads with Semaphores
