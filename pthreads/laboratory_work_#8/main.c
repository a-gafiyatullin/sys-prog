#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_STEPS 10000000
int stop_computations = 0;
unsigned long long int global_iter_counter = 0;
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
int threads_num;

void *pi_calc(void *arg);

void sig_act(int sig_num) { stop_computations = 1; }

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

  if (signal(SIGINT, sig_act) == SIG_ERR) {
    perror("signal");
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
  double *sum = malloc(sizeof(double));
  *sum = 0;
  unsigned long long int local_iter_counter = 0, i;

  while (1) {
    for (i = local_iter_counter * NUM_STEPS + start_i;
         i < (local_iter_counter + 1) * NUM_STEPS; i += threads_num) {
      *sum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }
    pthread_mutex_lock(&global_mutex);
    if (stop_computations) {
      if (global_iter_counter <= local_iter_counter) {
        pthread_mutex_unlock(&global_mutex);
        break;
      }
    } else {
      if (global_iter_counter < local_iter_counter) {
        global_iter_counter = local_iter_counter;
      }
    }
    pthread_mutex_unlock(&global_mutex);
    local_iter_counter++;
  }

  pthread_exit(sum);
}