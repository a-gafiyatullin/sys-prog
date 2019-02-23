#ifndef LINKED_LIST
#define LINKED_LIST

#include <stdlib.h>
#include <string.h>

struct node {
    char* data;
    struct node* next;
};

struct node* push_back(struct node* prev, char* str);

void erase(struct node* head);

#endif /* LINKED_LIST */
