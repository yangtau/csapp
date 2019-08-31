/*
 * mm.c - The fastest, least memory-efficient malloc package.
 *
 * TODO:
 * change some char* to void*
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"
team_t team = {
    /* Team name */
    "YT",
    /* First member's full name */
    "Tau Yang",
    /* First member's email address */
    "yangtaojay@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/*
allocated block
+------------+
| header     |
+------------+
|            |
/            /
|            |
+------------+

free block
+------------+
| header     |
+------------+
| prev       |
+------------+
| next       |
+------------+
|            |
/            /
|            |
+------------+
| footer     |
+------------+
*/

// #define DEBUG
#define VERBOSE 1

#ifdef DEBUG
#define DEBUG_RETURN(args)                              \
    {                                                   \
        mm_check_heap(__FUNCTION__, __LINE__, VERBOSE); \
        return args;                                    \
    }
#else  // DEBUG
#define DEBUG_RETURN(args) \
    { return args; }
#endif  // DEBUG

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Size of pointer */
#define PTR_SIZE (ALIGN(sizeof(char *)))

#define MINIMUM_BLOCK_SIZE (SIZE_T_SIZE * 2 + PTR_SIZE * 2)

/*
 * Given the size needed, complute the real size of
 * a block including header and padding
 */
#define REAL_SIZE(size) (ALIGN((size) + SIZE_T_SIZE))

/*
 * Pack size , allocated bit and previous allocated bit into a word.
 * @alloc and @prev_alloc must be 0 or 1
 */
#define PACK(size, alloc, prev_alloc) ((size) | (alloc) | ((prev_alloc) << 1))

/* Read and write data with type from address p*/
#define GET(p, type) (*(type *)(p))
#define PUT(p, type, val) (*(type *)(p) = (type)(val))

/* Read size and allocated fields from address p */
#define GET_SIZE(p) (GET((p), size_t) & ~0x7)
#define GET_ALLOC(p) (GET((p), size_t) & 0x1)

/* Read previous allocated field from address p */
#define GET_ALLOC_PREV(p) ((GET((p), size_t) & 0x2) >> 1)

/* Update size filed and preserve other fields */
#define UPDATE_SIZE(p, size) \
    (PUT((p), size_t, (GET((p), size_t) & 0x7) | (size)))

/* Update alloc filed and preserve other fields */
#define UPDATE_ALLOC(p, val) \
    (PUT((p), size_t, (GET((p), size_t) & ~0x1) | (val)))

/* Update previous alloc filed and preserve other fields */
#define UPDATE_ALLOC_PREV(p, val) \
    (PUT((p), size_t, (GET((p), size_t) & ~0x2) | ((val) << 1)))

/* Given block pointer bp, compute address of its header, footer*/
#define GET_HEAD(bp) ((char *)(bp)-SIZE_T_SIZE)
#define GET_FOOT(bp) ((char *)(bp) + GET_SIZE(GET_HEAD(bp)) - 2 * SIZE_T_SIZE)

/* Given block ptr bp, compute the address of next block */
#define GET_NEXT(bp) ((char *)(bp) + GET_SIZE(GET_HEAD(bp)))
/* Given block ptr bp, compute the address of previous block */
#define GET_PREV(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-2 * SIZE_T_SIZE))

/*
 * NOTE:
 * A block is previous or next block of another block means that their addresses
 * are joint. For the predecessor or the successor of a block, it means that
 * they are chained by pointer in the free list, and they are all free blocks.
 */

/* Given block ptr bp, compute the address of predecessor and successor */
#define GET_PRED(bp) (*(char **)(bp))
#define GET_SUCC(bp) (*(char **)((char *)(bp) + PTR_SIZE))

/* Given block ptr bp, compute the address of address predecessor */
#define GET_PRED_P(bp) ((char *)(bp))
/* Given block ptr bp, compute the address of address successor */
#define GET_SUCC_P(bp) ((char *)(bp) + PTR_SIZE)

/* Given header ptr hp, compute the address of block*/
#define GET_BLOCK(hp) ((char *)(hp) + SIZE_T_SIZE)

static char *mm_heap_start = NULL;

/*
 * Tools for debug:
 * print_free: print info about free blocks in free list
 * print_all: print all blocks
 */
static void print_free() {
    printf("----------------------------------------------------\n");
    printf("Free block list:\n");
    printf("%10s %5s %8s %8s\n", "addr", "size", "prev", "next");
    char *bp;
    for (bp = GET_BLOCK(mm_heap_start); bp != NULL; bp = GET_SUCC(bp)) {
        printf("%p %5ld %p %p\n", bp, GET_SIZE(GET_HEAD(bp)), GET_PRED(bp),
               GET_SUCC(bp));
    }
    printf("----------------------------------------------------\n");
}

static void print_all() {
    printf("----------------------------------------------------\n");
    printf("All blocks:\n%10s %5s %5s %5s\n", "addr", "size", "alloc",
           "prevalloc");
    char *bp = GET_BLOCK(mm_heap_start);
    while (1) {
        size_t size = GET_SIZE(GET_HEAD(bp));
        printf("%p %5ld %5lx %5lx\n", bp, size, GET_ALLOC(GET_HEAD(bp)),
               GET_ALLOC_PREV(GET_HEAD(bp)));
        bp = GET_NEXT(bp);
        if (!size)
            break;
    }
    printf("----------------------------------------------------\n");
}

// Given block ptr bp, insert it into free block list
static void free_list_insert(char *bp) {
    char *first_bp = GET_BLOCK(mm_heap_start);
    char *second_bp = GET_SUCC(first_bp);
    PUT(GET_SUCC_P(bp), char *, second_bp);
    PUT(GET_PRED_P(bp), char *, first_bp);
    if (second_bp != NULL)
        PUT(GET_PRED_P(second_bp), char *, bp);
    if (first_bp != NULL)
        PUT(GET_SUCC_P(first_bp), char *, bp);
}

// Given free block ptr bp, remove it from free block list
static void free_list_remove(char *bp) {
    char *predecessor = GET_PRED(bp);
    char *successor = GET_SUCC(bp);
    if (predecessor != NULL)
        PUT(GET_SUCC_P(predecessor), char *, successor);
    if (successor != NULL)
        PUT(GET_PRED_P(successor), char *, predecessor);
}

// Coalesce adjoint free blocks with @bp, and return the free block after
// coalescing.
static void *coalesce(char *bp) {
    // Coalesce with previous block
    while (!GET_ALLOC_PREV(GET_HEAD(bp))) {
        free_list_remove(bp);  // remove bp from the free list
        size_t new_size = GET_SIZE(GET_HEAD(bp));
        bp = GET_PREV(bp);
        new_size += GET_SIZE(GET_HEAD(bp));

        UPDATE_SIZE(GET_HEAD(bp), new_size);  // update size in header
        UPDATE_SIZE(GET_FOOT(bp), new_size);  // update size in footer
    }
    // Coalesce with next block
    while (!GET_ALLOC(GET_HEAD(GET_NEXT(bp)))) {
        char *next_bp = GET_NEXT(bp);
        free_list_remove(next_bp);

        size_t new_size = GET_SIZE(GET_HEAD(next_bp)) + GET_SIZE(GET_HEAD(bp));

        UPDATE_SIZE(GET_HEAD(bp), new_size);  // update size in header
        UPDATE_SIZE(GET_FOOT(bp), new_size);  // update size in footer
    }
    DEBUG_RETURN(bp);
}

// Extend heap and return the last block after extending
static void *extend_heap(size_t size) {
    char *bp = mem_sbrk(size);
    if (bp == (void *)-1) {
        return NULL;
    }
    // preserve the allocated bit for previous block
    int prev_alloc = GET_ALLOC_PREV(GET_HEAD(bp));
    PUT(GET_HEAD(bp), size_t, PACK(size, 0, prev_alloc));
    PUT(GET_FOOT(bp), size_t, PACK(size, 0, 0));

    // new epilogue header
    PUT(GET_HEAD(GET_NEXT(bp)), size_t, PACK(0, 1, 0));

    DEBUG_RETURN(coalesce(bp));
}

// Split free block, and insert the remind part into free list
static void split(char *bp, size_t size) {
    if (GET_SIZE(GET_HEAD(bp)) < size + MINIMUM_BLOCK_SIZE) {
        return;
    }
    size_t old_size = GET_SIZE(GET_HEAD(bp));
    UPDATE_SIZE(GET_HEAD(bp), size);
    // create new footer
    PUT(GET_FOOT(bp), size_t, PACK(size, 0, 0));

    char *next_bp = GET_NEXT(bp);
    PUT(GET_HEAD(next_bp), size_t, PACK(old_size - size, 0, 0));
    PUT(GET_FOOT(next_bp), size_t, PACK(old_size - size, 0, 0));

    free_list_insert(next_bp);

#ifdef DEBUG
    mm_check_heap(__func__, __LINE__, VERBOSE);
#endif
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    mem_init();
    mm_heap_start = mem_heap_lo();

    mem_sbrk(MINIMUM_BLOCK_SIZE + SIZE_T_SIZE);
    if (mem_sbrk == (void *)-1) {
        return -1;
    }

    char *bp = GET_BLOCK(mm_heap_start);
    // prologue block
    PUT(GET_HEAD(bp), size_t, PACK(MINIMUM_BLOCK_SIZE, 1, 1));
    PUT(GET_FOOT(bp), size_t, PACK(MINIMUM_BLOCK_SIZE, 1, 0));
    PUT(GET_PRED_P(bp), char *, NULL);
    PUT(GET_SUCC_P(bp), char *, NULL);
    // epilogue header
    PUT(GET_HEAD(GET_NEXT(bp)), size_t, PACK(0, 1, 1));
#ifdef DEBUG
    mm_check_heap(__func__, __LINE__, VERBOSE);
#endif
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    char *free_block = GET_BLOCK(mm_heap_start);
    // TODO: abs
    size_t real_size = REAL_SIZE(size);
    if (real_size < MINIMUM_BLOCK_SIZE) {
        real_size = MINIMUM_BLOCK_SIZE;
    }
    while (GET_SUCC(free_block) != NULL) {
        free_block = GET_SUCC(free_block);
        if (GET_SIZE(GET_HEAD(free_block)) >= real_size) {
            split(free_block, real_size);
            UPDATE_ALLOC(GET_HEAD(free_block), 1);
            UPDATE_ALLOC_PREV(GET_HEAD(GET_NEXT(free_block)), 1);
            free_list_remove(free_block);
            DEBUG_RETURN(free_block);
        }
    }

    // need to extend heap
    free_block = extend_heap(real_size);
    if (free_block == NULL) {
        // failed to extend heap
        return NULL;
    }

    free_list_remove(free_block);
    UPDATE_ALLOC(GET_HEAD(free_block), 1);
    UPDATE_ALLOC_PREV(GET_HEAD(GET_NEXT(free_block)), 1);
    DEBUG_RETURN(free_block);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    UPDATE_ALLOC(GET_HEAD(ptr), 0);
    PUT(GET_FOOT(ptr), size_t, PACK(GET_SIZE(GET_HEAD(ptr)), 0, 0));
    UPDATE_ALLOC(GET_HEAD(ptr), 0);
    UPDATE_ALLOC_PREV(GET_HEAD(GET_NEXT(ptr)), 0);
    free_list_insert(ptr);
    coalesce(ptr);
    DEBUG_RETURN();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    size_t old_size = GET_SIZE(GET_HEAD(ptr));

    // Case 1: `ptr` is NULL
    if (ptr == NULL) {
        DEBUG_RETURN(mm_malloc(size));
    }

    // Case 2: size == 0
    if (size == 0) {
        mm_free(ptr);
        DEBUG_RETURN(NULL);
    }

    // TODO: abstraction
    // align size
    size_t real_size = REAL_SIZE(size);
    if (real_size < MINIMUM_BLOCK_SIZE) {
        real_size = MINIMUM_BLOCK_SIZE;
    }

    // Case 3: size is smaller than or equal to the size before
    if (real_size <= old_size) {
        DEBUG_RETURN(ptr);
    }

    // Case 4: size if bigger than before
    void *newptr;
    size_t copySize = old_size;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    if (size < copySize)
        copySize = size;

    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    DEBUG_RETURN(newptr);
}

static void check_block(void *bp, const char *func, int line) {
    if ((size_t)bp % ALIGNMENT) {
        printf("Error: %p is not aligned.Func: %s,line: %d\n", bp, func, line);
    }
    if (!GET_ALLOC(GET_HEAD(bp)) &&
        GET_SIZE(GET_HEAD(bp)) != GET_SIZE(GET_FOOT(bp))) {
        printf(
            "Error: block %p, size in header %ld does not match it in footer "
            "%ld.Func: "
            "%s,line: %d\n",
            bp, GET_SIZE(GET_HEAD(bp)), GET_SIZE(GET_FOOT(bp)), func, line);
    }
}

void mm_check_heap(const char *func, int line, int verbose) {
    // check prologue header
    if (verbose) {
        print_all();
        print_free();
    }

    char *bp = GET_BLOCK(mm_heap_start);
    if (GET_SIZE(GET_HEAD(bp)) != MINIMUM_BLOCK_SIZE ||
        !GET_ALLOC(GET_HEAD(bp)) || !GET_ALLOC_PREV(GET_HEAD(bp))) {
        printf("Error: bad prologue header.Func: %s,line: %d\n", func, line);
    }
    for (; GET_SIZE(GET_HEAD(bp)); bp = GET_NEXT(bp)) {
        check_block(bp, func, line);
    }
    // check epilogue header
    if (GET_SIZE(GET_HEAD(bp)) != 0 || !GET_ALLOC(GET_HEAD(bp))) {
        printf("Error: bad epilogue header.Func: %s,line: %d\n", func, line);
    }
}