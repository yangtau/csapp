# Chapter 8 Exceptional Control Flow

## Exceptions

### Why Exceptions?

System must be able to react to changes in system state that are not captured by internal program variables and are not necessarily related to the execution of the program. For example, packets arrive at the network adapter and must be stored in memory.

An **Exception** is an abrupt change in the control in response to some change in the processor's state.

### Exception Handling

Every exception is assigned an unique nonnegative integer **exception number**. **Exception table** maps from exception number to the start address of the corresponding exception handler.

When the processor detects that an event has occurred, it determines the corresponding exception number. The processor then makes an indirect procedure call, through the entry in exception table, to the corresponding handler.

There are some differences between an exception and a procedure call:

- They all store the return address. However, for exceptions, the return address is either the current instruction or the next instruction, depending on the class of exception.
- The processor also pushes some additional processor state onto the stack that will be necessary to restart the interrupted program. For example, the EFLAGS in x86.
- All of these items are pushed onto the kernel's stack rather than onto the user's stack.
- Exception handlers run in kernel mode.

### Classes of Exceptions

| Class     | Cause                         | Async/sync | Return Address                      |
| --------- | ----------------------------- | ---------- | ----------------------------------- |
| Interrupt | Signal from I/O device        | Async      | Next instruction                    |
| Trap      | Intentional exception         | Sync       | Next instruction                    |
| Fault     | Potentially recoverable error | Sync       | Might return to current instruction |
| Abort     | Nonrecoverable error          | Sync       | Never returns                       |

## Processes

A process is an instance of a program in execution.

Each program in the system runs in the context of some process. The context consists of the state that the program needs to run correctly, which includes the program's code and date in memory, stack, contents of its general-purpose registers, program counter, environment variables, and the set of open file descriptors. The context of a process is adequate for restarting the process.

Processes provide application with two key abstraction:

- An independent logical control flow.
- A private address space.

### User and Kernel Mode

A process running in kernel mode can execute any instruction and access any memory location in the system.

A process in user mode is not allowed to execute privileged instructions such as halt the processor, initiate an I/O operation. Nor is it allowed to directly reference code or data in the kernel area of the address space.

The only way for the process to change from user mode to kernel mode is via an exception.

### Context Switches

The operating system kernel implements multitasking using context switch which is built on top of the lower-level exception.

Context switch includes 3 steps:

- saves the context of the current process
- restores the saved context of some previously preempted process
- passes control to this newly restored process

### Process Control in Linux

#### fork

- Call once, return twice.
- Concurrent execution.
- Duplicate but separate address spaces.
- Shared files.

```c
// return 0 to child
// return PID of child to parent
// -1 on error
pid_t fork(void);
```


#### wait

After a process terminated, it is kept in a terminated state until it is reaped by its parent. A terminated process that has not been reaped yet is called a *zombie*. 

When a parent process terminates, the *inti* process (with a PID of 1) becomes the adopted parent of any orphaned children.

```c
// Return PID of child if OK, 0 (if WNOHANG), or 1 on error
pid_t waitpid(pid_t pid, int *status, int options);
```

`pid`

- If `pid` > 0, then the wait set is the singleton child process with PID of `pid`.

- If `pid` = -1, then the wait set consists of all child processes.

#### execve

```c
int execve(const char *filename, const char *argv[], const char *envp[]);
// Call once and never returns unless there is an error
```

## Signals

A signal is a small message that notifies a process that an event of some type has occurred in the system.

### Sending Signals

The kernel sends a signal by updating some state in the context of the destination process.

```c
int kill(pid_t pid, int sig);
// If pid is 0, then the kill sends signal sig to every process in the process group of the calling preocess.
```

### Receiving Signals

The kernel checks the set of unblocked pending signals(pending & ~blocked) for a process p, when it switch p from kernel mode to user mode.
If the set is not empty, then the kernel forces p to receive a signal in the set. The receipt of the signal triggers some action by the process. After that, control passes back to the next instruction in the logical flow.

```c
typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);
// Return the old handler if OK, else SIG_ERR.
// The default actions of SIGSTOP and SIGKILL cannot be changed.
```

Signal handlers can be interrupted by other handlers, but the signal number of current handler will be blocked.

### Safe Signal Handling

Signal handlers are tricky because they can run concurrently with the main
program and with each other. 

Some tips about writing handler:

- Keep handlers simple.

- Call only async-signal-safe functions.

- Save and restore *errno* (if functions that may set *errno* are called).

- Protect accesses to shared global data structures by blocking all signalsi
  (handlers can be interrupted by other handler).  

-  Declare global variables with `volatile`. The key word `volatile` tells
   compiler to read variables in memory every time they are are referenced.

- Declare flags with `sig_atomic_t`.


Note:

- The existance of a pending singal merely indicates that at least one signal
  has arrived. Signals cannot be used to count the occurrence of events in other
  processes.

- Synchronizing flows to avoid nasty concurrency bugs.

- Using `sigsuspend` to explicitly wait for signals.

