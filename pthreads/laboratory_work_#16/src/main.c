#include "linked_list.h"
#include <errno.h>
#include <stdio.h> //printf, fgets, stdin

#define LINE_LEN 80

int main() {
  struct linked_list *list = l_init();
  pthread_t sort;
  struct sort_task task;
  task.list = list;
  task.delay = SORT_DELAY;
  task.sort_status = START;

  if (pthread_create(&sort, NULL, l_sort, &task) < 0) {
    perror("pthread");
    return errno;
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

  pthread_mutex_lock(&list->mutex);
  task.sort_status = STOP;
  pthread_mutex_unlock(&list->mutex);

  pthread_join(sort, NULL);
  l_erase(list);
  return 0;
}