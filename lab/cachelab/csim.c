#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cachelab.h"
/*
Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>
• -h: Optional help flag that prints usage info
• -v: Optional verbose flag that displays trace info
• -s <s>: Number of set index bits (S = 2 s is the number of sets)
• -E <E>: Associativity (number of lines per set)
• -b <b>: Number of block bits (B = 2 b is the block size)
• -t <tracefile>: Name of the valgrind trace to replay
*/
const char *OPT_STR = "hvs:E:b:t:";

static int set_index_bit = 0;
static int num_lines = 0;
static int block_bit = 0;
static int verbose_mode = 0;  // 0 is not verbose
static char *trace_file = NULL;

void debug_info(char *s, char *s2) {
#ifdef DEBUG
    printf("%s", s);
    if (s2 != NULL) {
        printf("%s\n", s2);
    }
#endif
}

void free_at_exit() {
    if (trace_file) {
        free(trace_file);
    }
}
void parse_opt(int argc, char *argv[]) {
    int opt;
    extern char *optarg;

    while ((opt = getopt(argc, argv, OPT_STR)) != -1) {
        switch (opt) {
        case 'h':
            debug_info("help", NULL);
            exit(0);
            break;
        case 'v':
            debug_info("verbose", NULL);
            verbose_mode = 1;
            break;
        case 's':
            debug_info("set index bits: ", optarg);
            // set_index_bit
            sscanf(optarg, "%d", &set_index_bit);
            break;
        case 'E':
            debug_info("number of lines per set: ", optarg);
            sscanf(optarg, "%d", &num_lines);
            break;
        case 'b':
            debug_info("block bit: ", optarg);
            sscanf(optarg, "%d", &block_bit);
            break;
        case 't':
            trace_file = malloc(strlen(optarg));
            strcpy(trace_file, optarg);
            atexit(free_at_exit);
            break;
        }
    }
}
/*
static int set_index_bit = 0;
static int num_lines = 0;
static int block_bit = 0;
static int verbose_mode = 0;  // 0 is not verbose
*/

typedef struct _cache_item {
    char valid;
    unsigned long tag;
    unsigned un_use;
} Item;

void sim() {
    FILE *f = fopen(trace_file, "r");
    char line[100];

    if (f == NULL) {
        fprintf(stderr, "Failed to open trace file: %s\n", trace_file);
        exit(1);
    }

    // | valid bit | tag | index |
    const size_t num_sets = 1 << set_index_bit;
    // const size_t set_size = (size_t)num_lines * sizeof(Item);
    Item **cache;
    cache = calloc(num_sets, sizeof(Item *));
    for (size_t i = 0; i < num_sets; i++) {
        cache[i] = calloc(num_lines, sizeof(Item));
    }

    unsigned long index_mask = ~(unsigned long)0;
    index_mask >>= (64 - set_index_bit);

    int tag_bit = 64 - set_index_bit - block_bit;
    unsigned long tag_mask = ~(unsigned long)0;
    tag_mask >>= (64 - tag_bit);

    char opt;
    unsigned long address;
    unsigned size;
    int hit_cnt = 0, miss_cnt = 0, eviction_cnt = 0;
    while (fgets(line, 100, f) != NULL) {
        if (line[0] == 'I') {
            continue;
        }
        sscanf(line, " %c %lx,%u", &opt, &address, &size);
        unsigned long index = (address >> block_bit) & index_mask;
        unsigned long tag = (address >> (block_bit + set_index_bit)) & tag_mask;

        // find in cache
        size_t i = 0;
        size_t lru_index = 0;
#define DEBUG
#ifdef DEBUG
        printf("tag:%8lx index:%8lx\t", tag, index);
#endif
        if (verbose_mode) {
            printf("%c %lx,%d ", opt, address, size);
        }

        int flag = 0;
        for (; i < num_lines; i++) {
            if (cache[index][i].valid && cache[index][i].tag == tag) {
                // found
                flag = 1;
                hit_cnt++;
                cache[index][i].un_use = 0;
                if (verbose_mode)
                    printf("%s", "hit");
                if (opt == 'M') {
                    hit_cnt++;
                    if (verbose_mode)
                        printf("%s", " hit");
                }
            } else {
                cache[index][i].un_use++;
            }
            if (!cache[index][i].valid) {
                lru_index = i;
                continue;
            }
            if (cache[index][lru_index].valid &&
                cache[index][i].un_use > cache[index][lru_index].un_use) {
                lru_index = i;
            }
        }
        if (flag == 0) {
            // not found
            miss_cnt++;
            if (verbose_mode)
                printf("%s", "miss");
            // eviction
            if (cache[index][lru_index].valid) {
                eviction_cnt++;
                if (verbose_mode)
                    printf("%s", " eviction");
            }
#ifdef DEBUG
            printf("[lru_index: %ld]", lru_index);
#endif

            cache[index][lru_index].tag = tag;
            cache[index][lru_index].valid = 1;
            cache[index][lru_index].un_use = 0;
            // cache[index][l]
            if (opt == 'M') {
                hit_cnt++;
                printf("%s", " hit");
            }
        }
        if (verbose_mode)
            printf("%s", " \n");
    }
    printSummary(hit_cnt, miss_cnt, eviction_cnt);
#undef DEBUG
}

int main(int argc, char *argv[]) {
    parse_opt(argc, argv);
    if (set_index_bit <= 0 || num_lines <= 0 || block_bit <= 0 ||
        trace_file == NULL) {
        exit(1);
    }
    sim();
    return 0;
}
