//
// Created by xp10rd on 2/10/19.
//

#include "linked_list.h"

struct node* push_back(struct node* prev, char* str) {
    struct node* new_node = malloc(sizeof(struct node));
    new_node->next = NULL;
    prev->next = new_node;

    new_node->data = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(new_node->data, str);

    return new_node;
}
