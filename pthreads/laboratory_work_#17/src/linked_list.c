#include "linked_list.h"

struct node *push_front(struct node *prev, char *str) {
  struct node *new_node = malloc(sizeof(struct node));
  if (new_node == NULL) {
    perror("push_front::malloc for node");
    return NULL;
  }
  if (pthread_mutex_init(&new_node->mutex, NULL)) {
    perror("push_front::pthread_mutex_init for node");
    free(new_node);
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
    pthread_mutex_lock(&head->mutex);
    struct node *next = head->next;
    free(head->data);
    pthread_mutex_unlock(&head->mutex);
    pthread_mutex_destroy(&head->mutex);
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
  struct node *new_head = push_front(list->head, str);
  pthread_mutex_lock(&list->mutex);
  list->head = new_head;
  pthread_mutex_unlock(&list->mutex);
}

void l_print(struct linked_list *list) {
  pthread_mutex_lock(&list->mutex);
  struct node *current = list->head;
  while (current) {
    pthread_mutex_lock(&current->mutex);
    if (write(STDOUT_FILENO, current->data,
              sizeof(char) * strlen(current->data)) < 0) {
      perror("l_print::write");
    }
    struct node *tmp = current;
    current = current->next;
    pthread_mutex_unlock(&tmp->mutex);
  }
  pthread_mutex_unlock(&list->mutex);
}

void swap_after(struct node *l_parent, struct node *r_parent) {
  if (l_parent == NULL || r_parent == NULL) {
    return;
  }

  struct node *l_child = l_parent->next;
  if (l_child == NULL) {
    return;
  }
  struct node *l_grand_child = l_child->next;

  struct node *r_child = r_parent->next;
  if (r_child == NULL) {
    return;
  }
  struct node *r_grand_child = r_child->next;

  l_child->next = r_grand_child;
  l_parent->next = r_child;
  r_child->next = l_grand_child;
  r_parent->next = l_child;
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
    if (task->sort_state == STOP) { // check if sort thread can be canceled
      pthread_mutex_unlock(&list->mutex);
      break;
    }
    pthread_mutex_unlock(&list->mutex);

    if (time(NULL) - prev_t > task->delay) { // time to sort
      pthread_mutex_lock(&list->mutex);
      if (list->head == NULL) { // check if nothing to sort
        pthread_mutex_unlock(&list->mutex);
        continue;
      }
      struct node *out = list->head;
      pthread_mutex_unlock(&list->mutex);

      pthread_mutex_lock(&out->mutex);
      struct node *out_next = out->next;
      pthread_mutex_unlock(&out->mutex);

      for (; out_next != NULL;) {
        struct node *in = out_next;

        pthread_mutex_lock(&in->mutex);
        struct node *in_next = in->next;
        pthread_mutex_unlock(&in->mutex);

        for (; in_next != NULL;) {         // sort list without first element
          pthread_mutex_lock(&out->mutex); // lock the leftmost element
          if (out->next !=
              in) { // try to lock child of the the leftmost element
            pthread_mutex_lock(&out->next->mutex);
          }
          pthread_mutex_lock(&in->mutex); // lock the right element
          pthread_mutex_lock(
              &in->next->mutex); // lock child of the right element

          if (strcmp(out->next->data, in->next->data) > 0) {
            if (out->next == in) { // case for nodes - neighbours
              swap_after_neighbours(out, in);
              in = out->next;
            } else {
              swap_after(out, in);
              pthread_mutex_unlock(&out->next->mutex);
            }
          } else if (out->next != in) {
            pthread_mutex_unlock(&out->next->mutex);
          }
          pthread_mutex_unlock(
              &in->next->mutex); // unlock mutexes in the reverse order
          pthread_mutex_unlock(&in->mutex);
          pthread_mutex_unlock(&out->mutex);

          in = in_next;
          pthread_mutex_lock(&in->mutex);
          in_next = in->next;
          pthread_mutex_unlock(&in->mutex);
        }

        out = out_next;
        pthread_mutex_lock(&out->mutex);
        out_next = out->next;
        pthread_mutex_unlock(&out->mutex);
      }

      pthread_mutex_lock(&list->mutex);
      struct node *first = list->head;
      pthread_mutex_unlock(&list->mutex);

      pthread_mutex_lock(&first->mutex);
      out_next = first->next;
      for (; out_next != NULL;) { // look up a new position for the top element
        pthread_mutex_lock(&first->next->mutex);
        if (strcmp(list->head->data, first->next->data) < 0) {
          if (list->head == first) { // element is already in the right place
            pthread_mutex_unlock(&first->next->mutex);
            break;
          }
          struct node *new_head = list->head->next;
          list->head->next = first->next;
          first->next = list->head;
          list->head = new_head;
          pthread_mutex_unlock(&first->next->next->mutex);
          pthread_mutex_unlock(&first->next->mutex);
          break;
        }
        if (first != list->head) {
          pthread_mutex_unlock(&first->mutex);
        }
        first = out_next;
        out_next = first->next;
      }
      pthread_mutex_unlock(&first->mutex);
      if (out_next == NULL) {
        if (first != list->head) {
          pthread_mutex_unlock(&list->head->mutex);
        }
      }

      pthread_mutex_lock(&list->mutex);
      if (out_next == NULL &&
          first != list->head) { // top element is the biggest from all
        pthread_mutex_lock(&list->head->mutex);
        first->next = list->head;
        list->head = list->head->next;
        first->next->next = NULL;
        pthread_mutex_unlock(&first->next->mutex);
      }
      pthread_mutex_unlock(&list->mutex);

      prev_t = time(NULL);
    }
  }

  pthread_exit(NULL);
}