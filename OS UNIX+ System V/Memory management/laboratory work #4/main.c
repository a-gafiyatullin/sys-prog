#include <stdio.h>
#include "linked_list.h"

int main() {

    char line[BUFSIZ];
    struct node *head = malloc(sizeof(struct node));
    struct node *current = head;
    while(fgets(line, BUFSIZ, stdin)) {
        if(line[0] == '.') {
            break;
        } else {
            current = push_back(current, line);
        }
    }

    current = head->next;
    while(current) {
        printf("%s", current->data);
        current = current->next;
    }

    return 0;
}