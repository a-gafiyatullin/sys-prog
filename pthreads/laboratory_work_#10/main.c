#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MUTEX_COUNT 3
#define ITERATIONS 4

pthread_mutex_t m[MUTEX_COUNT];
int flag = 0;

void *print_message(void *str) {
  int i = 0, k = 1;

  pthread_mutex_lock(&m[2]);
  if (flag)
    pthread_mutex_unlock(&m[0]);
  for (i = 0; i < ITERATIONS * MUTEX_COUNT; i++) {
    pthread_mutex_lock(&m[k]);
    k = (k + 1) % 3;
    pthread_mutex_unlock(&m[k]);
    if (k == 2) {
      write(STDOUT_FILENO, str, strlen(str));
      flag = 1;
    }
    k = (k + 1) % 3;
  }
  pthread_mutex_unlock(&m[2]);
  pthread_exit(NULL);
}

int main() {
  pthread_t pthread;
  int i = 0;

  for (i = 0; i < MUTEX_COUNT; i++)
    pthread_mutex_init(&m[i], NULL);

  pthread_mutex_lock(&m[0]);
  if (pthread_create(&pthread, NULL, print_message, "child\n") < 0) {
    fprintf(stderr, "Error in creating thread!\n");
    pthread_exit(NULL);
  }

  while (!flag) {
    sched_yield();
  }

  print_message("parent\n");

  pthread_join(pthread, NULL);
  for (i = 0; i < MUTEX_COUNT; i++)
    pthread_mutex_destroy(&m[i]);

  pthread_exit(NULL);
}
