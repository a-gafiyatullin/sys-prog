#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PRINTING_AMOUNT 10
#define SLEEP_TIME 1

void *print_text(void *data) {
  for (int i = 0; i < PRINTING_AMOUNT; i++) {
    printf("Print some text #%d\n", i);
    sleep(SLEEP_TIME);
  }

  pthread_exit(data);
}

int main() {
  pthread_t thread;

  if (pthread_create(&thread, NULL, print_text, NULL) != 0) {
    fprintf(stderr, "Error of the thread creation!\n");
    exit(-1);
  }
  sleep(SLEEP_TIME * 2);
  if (pthread_cancel(thread) != 0) {
    fprintf(stderr, "Error of the thread cancelling!\n");
    exit(-1);
  }
  sleep(SLEEP_TIME * 2);
  printf("Here main thread\n");

  pthread_exit(NULL);
}