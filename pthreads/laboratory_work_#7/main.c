#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_STEPS 2000000000
int threads_num;

void *pi_calc(void *arg);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: pi-serial positive_number_of_threads\n");
    exit(-1);
  }
  threads_num = atoi(argv[1]);
  if (threads_num < 0) {
    fprintf(stderr, "usage: pi-serial positive_number_of_threads\n");
    exit(-1);
  }
  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  if (threads == NULL) {
    perror("malloc");
    exit(-1);
  }

  for (int i = 0; i < threads_num; i++) {
    if (pthread_create(threads + i, NULL, pi_calc, (void *)i) < 0) {
      fprintf(stderr, "Error of the thread creation!\n");
      exit(-1);
    }
  }

  double sum = 0;
  double *ret_val;
  for (int i = 0; i < threads_num; i++) {
    if (pthread_join(threads[i], (void **)&ret_val) < 0) {
      fprintf(stderr, "Error of the thread joining!\n");
      exit(-1);
    }
    sum += *((double *)ret_val);
    free(ret_val);
  }

  printf("pi = %.15g\n", 4.0 * sum);
  free(threads);
  exit(0);
}

void *pi_calc(void *arg) {
  int start_i = (int)arg;
  unsigned long long int i = 0;
  double *sum = malloc(sizeof(double));
  *sum = 0;

  for (i = start_i; i < NUM_STEPS; i += threads_num) {
    *sum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
  }

  pthread_exit(sum);
}