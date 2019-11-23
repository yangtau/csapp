#include "csapp.h"
#include "queue.h"
#define THREAD_NUM 40
#define N_LEN 1000000

struct queue que;

void *thread(void *vargp) {
  int n = N_LEN / THREAD_NUM;
  long sum = 0;
  while (n--) {
    int a = queue_get(&que);
    // printf("%d\n", a);
    sum += a;
  }
  return sum;
}

int main() {
  pthread_t tids[THREAD_NUM];

  queue_init(&que, 100);


  for (int i = 0; i < THREAD_NUM; i++) {
    Pthread_create(&tids[i], NULL, thread, NULL);
  }

  long e = 0;
  for (int i = 0; i < N_LEN; i++) {
    queue_put(&que, i);
    e += i;
  }

  long res = 0;
  for (int i = 0; i < THREAD_NUM; i++) {
    long s = 0;
    Pthread_join(tids[i], &s);
    res += s;
  }

  printf("r:%ld\n", res);
  printf("e:%ld\n", e);
  queue_free(&que);
  return 0;
}
