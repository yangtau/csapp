/*
 * A producer-consumer queue.
 *
 * Multiple threads can get items from a queue concurrently. However, only one
 * thread can put a item into the queue in any time. That is to say, there is
 * only one producer at the same time, but there can be multiple consumers.
 *
 * If the queue is empty, then consumers will be suspended until the producer
 * puts a item into the queue. Meanwhile, if the queue is full (the queue is in
 * fixed), the producer will be suspended until any consumer take a item from
 * the queue.
 */
#include "csapp.h"

struct queue {
  int size;
  int *buf;
  int front; /* buf[front%size] is the first item */
  int rear;  /* buf[(rear-1)%size] is the last item */
  sem_t consumer_mutex;
  sem_t slots; /* notify the producer for slots */
  sem_t items; /* notify consumers for items */
};

/* initialize the que with its size */
void queue_init(struct queue *que, int size);

/* free the que */
void queue_free(struct queue *que);

/* get and remove the front item in the que */
int queue_get(struct queue *que);

/* put the item in the end of the que */
void queue_put(struct queue *que, int item);
