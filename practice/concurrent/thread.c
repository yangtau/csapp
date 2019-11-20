#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread(void *argp) {
  int id = (int)argp;
  static int cnt = 0;
  printf("id: %d, cnt: %d\n", id, cnt++);
  return NULL;
}

void sys_error(char *msg) {
  printf("%s\n", msg);
  exit(-1);
}

int main() {
  pthread_t tid;
  for (int i = 0; i < 100; i++) {
    if (pthread_create(&tid, NULL, thread, (void *)i) != 0) {
      sys_error("Error: main pthread_create");
    }
  }
  pthread_exit(NULL);
  return 0;
}
