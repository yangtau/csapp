#include "queue.h"
#include "csapp.h"

void queue_init(struct queue *que, int size) {
  que->size = size;
  que->buf = (int *)malloc(sizeof(int) * size);
  que->front = 0;
  que->rear = 0;

  sem_init(&que->consumer_mutex, 0, 1);
  sem_init(&que->slots, 0, size);
  sem_init(&que->items, 0, 0);
}

void queue_free(struct queue *que) {
  free(que->buf);

  sem_destroy(&que->consumer_mutex);
  sem_destroy(&que->slots);
  sem_destroy(&que->items);
}

int queue_get(struct queue *que) {
  int item;

  P(&que->items);
  P(&que->consumer_mutex);
  item = que->buf[que->front];
  que->front = (que->front + 1) % que->size;
  V(&que->consumer_mutex);
  V(&que->slots);

  return item;
}

void queue_put(struct queue *que, int item) {
  P(&que->slots);
  que->buf[que->rear++] = item;
  V(&que->items);

  que->rear %= que->size;
}
