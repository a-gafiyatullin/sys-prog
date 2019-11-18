#ifndef LINKED_LIST
#define LINKED_LIST

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SORT_DELAY 5
#define START 0
#define STOP 1

struct node {
  char *data;
  struct node *next;
};

struct linked_list {
  pthread_mutex_t mutex;
  struct node *head;
};

struct sort_task {
  struct linked_list *list;
  int delay;
  int sort_status;
};

struct linked_list *l_init();

void l_print(struct linked_list *list);

void *l_sort(void *arg);

void l_push_front(struct linked_list *list, char *str);

void l_erase(struct linked_list *list);

#endif /* LINKED_LIST */
