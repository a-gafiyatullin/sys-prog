#include <stdio.h> //printf, fgets, stdin
#include <unistd.h> //sysconf
#include "linked_list.h"

int main() {
    int line_max = sysconf(_SC_LINE_MAX);
    char* line = malloc(line_max * sizeof(char));
    struct node *head = malloc(sizeof(struct node));
    struct node *current = head;
    while(fgets(line, line_max, stdin)) {
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

    erase(head);
    free(line);
    return 0;
}