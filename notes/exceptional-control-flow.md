# Chapter 8 Exceptional Control Flow

## Exceptions

### Why Exceptions?

System must be able to react to changes in system state that are not captured by internal program variables and are not necessarily related to the execution of the program. For example, packets arrive at the network adapter and must be stored in memory.

An **Exception** is an abrupt change in the control in response to some change in the processor's state.

### Exception Handling

Every exception is assigned a unique nonnegative integer **exception number**. **Exception table** maps from exception number to the start address of the corresponding exception handler.

When the processor detects that an event has occurred and determines the corresponding exception number. The processor then makes an indirect procedure call, through the entry in exception table, to the corresponding handler.

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

Each program in the system runs in the context of some process. The context consists of the state that the program needs to run correctly, which includes the program's code and date in memory, stack, contents of its general-purpose registers, program counter, enviornment variables, and the set of open file descriptors. The context of a process is adequate for restarting the process.

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

## Signals

A signal is a small message that notifies a process that an event of some type has occurred in the system.

### Sending Signals

The kernel sends a signal by updating some state in the context of the destination process.

### Receiving Signals

The kernel checks the set of unblocked pending signals(pending & ~blocked) for a process p, when it switch p from kernel mode to user mode.
If the set is not empty, then the kernel forces p to receive a signal in the set. The receipt of the signal triggers some action by the process. After that, control passes back to the next instruction in the logical flow.

