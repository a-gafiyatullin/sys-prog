#include "linked_list.h"
#include <errno.h>
#include <stdio.h>

#define LINE_LEN 80
#define SORT_THREADS 3

int main() {
  struct linked_list *list = l_init();
  pthread_t sort_threads[SORT_THREADS];
  struct sort_task tasks[SORT_THREADS];
  int delays[SORT_THREADS] = {1, 4, 8};

  for (int i = 0; i < SORT_THREADS; i++) {
    tasks[i].list = list;
    tasks[i].delay = delays[i];
    tasks[i].sort_state = START;
    if (pthread_create(sort_threads + i, NULL, l_sort, tasks + i) < 0) {
      perror("pthread");
      return errno;
    }
  }
  char line[LINE_LEN];
  while (fgets(line, LINE_LEN, stdin)) {
    if (line[0] == '.') {
      break;
    } else if (line[0] == '\n') {
      l_print(list);
    } else {
      l_push_front(list, line);
    }
  }

  for (int i = 0; i < SORT_THREADS; i++) {
    pthread_mutex_lock(&tasks[i].list->mutex);
    tasks[i].sort_state = STOP;
    pthread_mutex_unlock(&tasks[i].list->mutex);
  }
  for (int i = 0; i < SORT_THREADS; i++) {
    pthread_join(sort_threads[i], NULL);
  }

  l_erase(list);
  return 0;
}