#include "cache.h"
#include <time.h>

static const char *ERROR_MSG[] = {
    "OK",                                     /* */
    "cache_put: object oversize",             /* error code: 1 */
    "cache_put: evict noting",                /* error code: 2 */
    "cache_get: no corresponding item found", /* error code: 2 */
};


const char *cache_error_msg(int error_code) {
  return ERROR_MSG[error_code];
}

struct cache_item {
  int size;
  time_t last_used_time;
  char *id;
  char *data;
  struct cache_item *next;
  struct cache_item *prev;
};

static struct cache_item *__cache_item_init(const char *id, const char *data,
                                            int data_len) {
  struct cache_item *item =
      (struct cache_item *)malloc(sizeof(struct cache_item));

  item->prev = NULL;
  item->next = NULL;

  item->size = data_len;
  item->id = strdup(id);
  item->data = (char *)malloc(data_len);

  memcpy(item->data, data, data_len);

  time(&item->last_used_time);

  return item;
}

static void __cache_item_free(struct cache_item *item) {
  if (item->id) {
    free(item->id);
  }
  if (item->data) {
    free(item->data);
  }
}

int cache_init(struct cache *cache) {
  cache->reader_cnt = 0;
  cache->total_size = 0;
  cache->items_list = NULL;
  sem_init(&cache->reader_cnt_mutex, 0, 1);
  sem_init(&cache->w_mutex, 0, 1);
  return 0;
}

/* __cache_item_evict: evict item using least_recently_used policy */
static int __cache_item_evict(struct cache *cache) {
  time_t earliest_time;
  struct cache_item *lru, *p;

  lru = cache->items_list;
  if (lru != NULL) {
    earliest_time = lru->last_used_time;
  }

  for (p = cache->items_list; p; p = p->next) {
    if (p->last_used_time < earliest_time) {
      earliest_time = p->last_used_time;
      lru = p;
    }
  }

  if (lru == NULL) {
    return 2; /* no item can be evict */
  }

  /* remove item from the list */
  if (lru->next != NULL) {
    lru->next->prev = lru->prev;
  }
  if (lru->prev != NULL) {
    lru->prev->next = lru->next;
  }

  __cache_item_free(lru);

  return 0;
}

int cache_put(struct cache *cache, const char *id, const char *data,
              int data_len) {
  int rc = 0;

  if (data_len > MAX_OBJECT_SIZE) {
    rc = 1;
    goto on_error;
  }

  P(&cache->w_mutex);

  while (data_len + cache->total_size > MAX_CACHE_SIZE) {
    __cache_item_evict(cache);
  }

  /* create and insert new item node */
  struct cache_item *item = __cache_item_init(id, data, data_len);

  item->next = cache->items_list;
  if (cache->items_list != NULL) {
    cache->items_list->prev = item;
  }

  cache->items_list = item;

  V(&cache->w_mutex);

on_error:
  return rc;
}

int cache_get(struct cache *cache, const char *id, char *buf, int max_buf_len,
              int *len) {
  int rc = 0;
  struct cache_item *p;

  P(&cache->reader_cnt_mutex);
  if (cache->reader_cnt == 0) {
    P(&cache->w_mutex);
  }
  cache->reader_cnt++;
  V(&cache->reader_cnt_mutex);

  for (p = cache->items_list; p != NULL; p = p->next) {
    if (strcasecmp(id, p->id) == 0) {
      *len = p->size < max_buf_len ? p->size : max_buf_len;
      memcpy(buf, p->data, *len);

      time(&p->last_used_time);
      break;
    }
  }

  if (p == NULL) {
    rc = 3;
  }

  P(&cache->reader_cnt_mutex);
  cache->reader_cnt--;
  if (cache->reader_cnt == 0) {
    V(&cache->w_mutex);
  }
  V(&cache->reader_cnt_mutex);

  return rc;
}
