#include "linked_list.h"

struct node *push_front(struct node *prev, char *str) {
  struct node *new_node = malloc(sizeof(struct node));
  if (new_node == NULL) {
    perror("push_front::malloc for node");
    return NULL;
  }
  new_node->next = prev;

  new_node->data = malloc(sizeof(char) * (strlen(str) + 1));
  if (new_node->data != NULL) {
    strcpy(new_node->data, str);
  } else {
    perror("push_front::malloc for node data");
  }

  return new_node;
}

void erase(struct node *head) {
  while (head != NULL) {
    struct node *next = head->next;
    free(head->data);
    free(head);
    head = next;
  }
}

struct linked_list *l_init() {
  struct linked_list *list = malloc(sizeof(struct linked_list));
  if (list == NULL) {
    perror("l_init::malloc");
    return NULL;
  }
  if (pthread_mutex_init(&list->mutex, NULL) < 0) {
    free(list);
    perror("l_init::pthread_mutex_init");
    return NULL;
  }
  list->head = NULL;

  return list;
}

void l_erase(struct linked_list *list) {
  if (list == NULL) {
    return;
  }
  pthread_mutex_lock(&list->mutex);
  erase(list->head);
  pthread_mutex_unlock(&list->mutex);
  pthread_mutex_destroy(&list->mutex);
  free(list);
}

void l_push_front(struct linked_list *list, char *str) {
  pthread_mutex_lock(&list->mutex);
  list->head = push_front(list->head, str);
  pthread_mutex_unlock(&list->mutex);
}

void l_print(struct linked_list *list) {
  pthread_mutex_lock(&list->mutex);
  struct node *current = list->head;
  while (current) {
    if (write(STDOUT_FILENO, current->data,
              sizeof(char) * strlen(current->data)) < 0) {
      perror("l_print::write");
    }
    current = current->next;
  }
  pthread_mutex_unlock(&list->mutex);
}

void swap_after(struct node *l_parent, struct node *r_parent) {
  if (l_parent == NULL || r_parent == NULL) {
    return;
  }
  struct node *l_child = l_parent->next;
  struct node *r_child = r_parent->next;
  if (l_child == NULL || r_child == NULL) {
    return;
  }
  struct node *l_grand_child = l_child->next;
  struct node *r_grand_child = r_child->next;
  l_parent->next = r_child;
  r_child->next = l_grand_child;
  r_parent->next = l_child;
  l_child->next = r_grand_child;
}

void swap_after_neighbours(struct node *l_parent, struct node *r_parent) {
  if (l_parent->next != r_parent) {
    return;
  }
  l_parent->next = r_parent->next;
  r_parent->next = l_parent->next->next;
  l_parent->next->next = r_parent;
}

void *l_sort(void *arg) {
  struct sort_task *task = arg;
  struct linked_list *list = task->list;
  time_t prev_t = time(NULL);

  while (1) {
    pthread_mutex_lock(&list->mutex);
    if (task->sort_status == STOP) {
      pthread_mutex_unlock(&list->mutex);
      break;
    }
    pthread_mutex_unlock(&list->mutex);
    if (time(NULL) - prev_t > SORT_DELAY) {
      pthread_mutex_lock(&list->mutex);
      if (list->head != NULL) {
        for (struct node *out = list->head; out->next != NULL;
             out = out->next) {
          for (struct node *in = out->next; in->next != NULL; in = in->next) {
            if (strcmp(out->next->data, in->next->data) > 0) {
              if (out->next == in) { // case for nodes - neighbours
                swap_after_neighbours(out, in);
                in = out->next;
              } else {
                swap_after(out, in);
              }
            }
          }
        }
        struct node *first;
        for (first = list->head; first->next != NULL; first = first->next) {
          if (strcmp(list->head->data, first->next->data) < 0) {
            if (first->next ==
                list->head->next) { // case for nodes - neighbours
              break;
            }
            struct node *new_head = list->head->next;
            list->head->next = first->next;
            first->next = list->head;
            list->head = new_head;
            break;
          }
        }
        if (first->next == NULL) { // top element is the biggest from all
          first->next = list->head;
          list->head = list->head->next;
          first->next->next = NULL;
        }
      }
      pthread_mutex_unlock(&list->mutex);
      prev_t = time(NULL);
    }
  }

  pthread_exit(NULL);
}