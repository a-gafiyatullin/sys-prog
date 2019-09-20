#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *print_text(void *arg) {
  printf("1. It's the child thread!\n");
  printf("2. It's the child thread!\n");
  printf("3. It's the child thread!\n");
  printf("4. It's the child thread!\n");
  printf("5. It's the child thread!\n");

  pthread_exit(arg);
}

int main() {
  pthread_t thread;

  if (pthread_create(&thread, NULL, print_text, NULL) != 0) {
    fprintf(stderr, "Error of the thread creation!\n");
    exit(-1);
  }

  printf("6. It's the parent thread!\n");
  printf("7. It's the parent thread!\n");
  printf("8. It's the parent thread!\n");
  printf("9. It's the parent thread!\n");
  printf("10. It's the parent thread!\n");

  pthread_exit(NULL);
}
