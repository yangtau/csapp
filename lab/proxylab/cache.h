/*
 * A reader-writer cache.
 */
#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cache_item;

struct cache {
  int total_size;
  int reader_cnt;
  sem_t reader_cnt_mutex;
  sem_t w_mutex;
  struct cache_item *items_list;
};

const char *cache_error_msg(int error_code);

int cache_init(struct cache *cache);

int cache_free(struct cache *cache);

int cache_put(struct cache *cache, const char *id, const char *data,
              int data_len);

int cache_get(struct cache *cache, const char *id, char *buf, int max_buf_len, int *len);

#endif /* __CACHE_H__ */
