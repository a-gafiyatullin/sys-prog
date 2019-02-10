//
// Created by xp10rd on 2/10/19.
//

#ifndef LABORATORY_WORK_4_LINKED_LIST_H
#define LABORATORY_WORK_4_LINKED_LIST_H

#include <stdlib.h>
#include <string.h>

struct node {
    char* data;
    struct node* next;
};

struct node* push_back(struct node* prev, char* str);

#endif //LABORATORY_WORK_4_LINKED_LIST_H
