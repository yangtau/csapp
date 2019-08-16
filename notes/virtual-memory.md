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


