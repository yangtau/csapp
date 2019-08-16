# Chapter 9 Virtual Memory

## Concepts

Process provides each program with two key abstraction:

- Logical control flow
- Private address space

Every process has it own virtual address space, and one process cannot access other's space.

### Caching

Conceptually, virtual memory is an array of N contiguous bytes stored on disk.

Disk is about 10000 times slower than DRAM, so DRAM cache organization driven by the enormous miss penalty. 

- Virtual pages tend to be large.
- Fully associated: any virtual page can be placed in any physical page.
- Write-back instead write-through.

Virtual address need to be translated into physical address by *MMU*(memory management unit), and the data structure storing mapping from virtual address to physical address is called *page table*.

Page table is an array of *page table entries*(PTEs) which consists of *valid bit*, n-bit address field, pointer to file in disk, and maybe some other information.

*Page hit*: reference to VM word that is in physical memory.

*Page fault*: reference to VM word that is not in physical memory.

Transferring a page between disk and memory is known as *swapping* or *paging*. The strategy of waiting until the last moment to swap in a page, when a miss occurs, is known as *demand paging*.

### Memory Management

VM greatly simplified memory management. OS provide a separate page table, and thus **a separate virtual address space**, for each process. Moreover, multiple virtual pages can be mapped to the same shared physical page.

- Simplifying loading. Loader allocates virtual pages for code and data segments, marks them as invalid, and points their PTEs to the appropriate position in the object file. Due to demand paging, the loader never really copies any data from disk into memory.
- Simplifying sharing. Multiple processes shared a single copy of shared libraries by mapping the appropriate virtual pages in different processes to the same physical pages.
- Simplifying allocation. If a process ask for large contiguous memory which can be allocated directly from physical memory, OS simply maps contiguous virtual pages to discontiguous physical pages.

### Memory protection

VM provide a natural way to protect memory. First of all, providing separate virtual address space make it easy to isolate private memories of different processes. Moreover, to control access to a virtual page, we can add some additional permission bits to the PTE. Every time CPU genrates an address, MMU can check if the access is legal.

## Address Translation

$$
MAP: VAS \rightarrow PAS \cup \emptyset
$$

### Basic

```
            Virtual Address
            +------+------+
        +---+ VPN  | VPO  |
        |   +------+---+--+
        v              |
    +---+--------+     |
    | Page Table |     |
    +---------+--+     |
              |        |
              v        v
            +-+----+---+--+
            | PPN  | PPO  |
            +------+------+
            Physical Address

```
**VPN** : virtual page number.

**VPO**: virtual page offset.

**PPN**: physical page number.

**PPO**: physical page offset.

### TLB

Page table is stored in memory, thus every time the translation from VMA to PMA needs to read the PTE in physical memory first. If the PTE is not cached in cache, the cost is expensive. To eliminate this cost, many system add a small cache of PTEs in MMU called a **translation lookaside buffer**.

TLB is set-associative  hardware cache, and it maps  VPN to PPN.

```
+-------+-------+------+
| TLBT  | TLBI  | VPO  |
+-------+-------+------+
|<-----VPN----->|

*TLBT*: TLB tag
*TLBI*: TLB index
```

TLB needs to be clear, when switch from a process to another process. In practice, we add a filed in TPEs to distinguish different processes.

### Multi-Level Page Tables

Virtual Memory can be very large, thus the page table will be very large too. The common approach is to use hierarchy of page table.

```
+--------+--------+--------+-----+
| VP1    | VP2    | VP3    | VPO |
+---+----+---+----+---+----+--+--+
    |        |        |       |
    V        V        V       |
 +----+   +----+   +----+     |
 | L1 +-->| L2 +-->| L3 |     |
 +----+   +----+   +--+-+     |
                      |       |
                      V       V
        +-------------+-----+-+---+
        |     PPN           | PPO |
        +-------------------+-----+
```

If a PTE in the level 1 table is null, then the corresponding level 2 page table does not have to exist.
Only the level 1 table needs to be in main memory at all times. Page tables in other level can be created and paged in and out by the VM system as they are needed.

### Summary

TLB is to reduce time cost of VM, and multi-level page table is to reduce the memory cost of VM.

#### Trick for speeding up

```
PA
  +-------------------------------------+
  |                                     V
+-------+------+-------+         +------------+       
| CT    |  CI  |  CO   |         |  Tag Check |       
+-------+------+-------+         +------------+
+-------+--------------+           ^  ^  ^  ^
| PPN   |    PPO       |           |  |  |  |
+---+---+---------+----+         +------------+
    ^             ^              |   Cache    | 
    |             |              +-------+----+  
 Address          | no                   ^
Translation       | change               |
    |             |                      | read line
+---+---+---------+----+                 |
| VPN   |    VPO       |-----------------+
+-------+--------------+
VA
```

We can index into cache while address translation taking place. After that, we check the PPN with the tag in cache.

## Core i7 / Linux VM System

### Linux Virtual Memory Areas

Linux organizes the virtual memory as a collection of areas. An area is a contiguous chunk of existing virtual memory whose pages are related in some way. For example, code segment and data segment are distinct areas. 

In the paging fault exception handling, the handler will first check if the virtual address is in one of the areas.


## Memory Mapping

Linux initializes the contents of a virtual memory area by associating it with an object on disk. Areas can be mapped to one of two types of objects:
 - Regular file in the Linux files system: 
    The file section is divided into page-size pieces, with each piece containing the initial content of a virtual page.
    
 - Anonymous file.
   

An object can be mapped into an area of virtual memory as either a shared object or a private object.
Private object are mapped into virtual memory using a clever technique called **copy-on-write**. A private object begins life in the same way as shared object. The PTEs for the corresponding private area are flagged as read-only, and the area is flagged as private copy-on-write. If one process attempts to write to some of the shared private object, then the write triggers a protection fault.The fault handler creates a new copy of the page in physical memory, and updates the PTE to point to the new copy.

## Dynamic Memory Allocation

### The *malloc* Package

**calloc**: initializes allocated block to zero.
**sbrk**: grow and shrink heap

### Allocator Goals

Two goals:
- **Maximizing Throughput**
- **Maximizing Memory Utilization**

It is hard too to find an appropriate balance between the two goals.

#### Fragmentation

The primary cause of poor heap utilization.

Two forms:
- internal fragmentation:
    An allocated block is larger than the payload.
    Reason: alignment constraints or minimum size limit of blocks.
- external fragmentation:
    There is enough aggregate free memory, but no single free block is large enough to handle the request.

### Implementation

#### Issue

- **Free block organization**
- **Placement**: choose an appropriate free block
- **Splitting**: reminder of the free block after allocating.
- **Coalescing**: reinsert freed block.


#### Implicit free list

  using length to implicitly link all blocks.

**Finding a Free Block**

- First fit: from beginning, choose the first free blocks that fits.
- Next fit: like first fit, but start where previous search finished.
- Best fit: choose the best free blocks with fewest bytes left over

**Coalescing Free Blocks**

Merge adjacent free blocks.

**Boundary Tags**

If there is only a header of a block, then we cannot merge a free block with its previous  free block. The idea of *boundary tags* is to add a footer at the end of each block, such that single lists become dual lists.

There is a optimization of boundary tags that eliminates the need for a footer in allocated blocks. However, we still need to know that whether the previous block is free or allocated, so we store the free/allocated bit of previous block in the header of current block.


#### Explicit free list

- Maintain explicit list for all free blocks. 

- Still need boundary tags for coalescing.

Allocate is linear time in number of free blocks, and it is much faster when most of the memory is full.

#### Segregated free list

 Different free lists for different size classes. When the allocator needs a block of size n, it searches the appropriate free list first. If it cannot find one that fits, than it searches the next list, and so on.

- Segregated fits

  Each list contains potentially different -size blocks whose sizes are members of size classes.

- Buddy System

#### Blocks sorted by size

  using a balanced tree with pointers within each free blocks, and the length used as a key.


