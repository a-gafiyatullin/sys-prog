#include <pthread.h>
#include <stdio.h>

#define THREAD_POOL_SIZE 4

typedef struct string_seq {
  unsigned int size;
  char **strings;
} string_seq;

void *print_text(void *arg) {
  string_seq *seq = (string_seq *)arg;

  for (unsigned int i = 0; i < seq->size; i++) {
    printf("%s\n", seq->strings[i]);
  }

  pthread_exit(arg);
}

int main() {
  pthread_t threads[THREAD_POOL_SIZE];
  string_seq data[THREAD_POOL_SIZE];

  /*----------------- EXAMPLE OF THE USAGE(A BAD CODE) ---------------------*/
  char *strings[] = {"String №1", "String №2", "String №3", "String №4",
                     "String №5", "String №6", "String №7", "String №8"};
  data[0].size = 2;
  data[0].strings = strings;
  data[1].size = 3;
  data[1].strings = strings + 2;
  data[2].size = 1;
  data[2].strings = strings + 5;
  data[3].size = 2;
  data[3].strings = strings + 6;
  /*------------------------------------------------------------------------*/

  for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    if (pthread_create(threads + i, NULL, print_text, data + i) != 0) {
      fprintf(stderr, "Error of the thread creation!\n");
      return -1;
    }
  }

  pthread_exit(NULL);
}