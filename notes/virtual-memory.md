# Chapter 9 Virtual Memory

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


