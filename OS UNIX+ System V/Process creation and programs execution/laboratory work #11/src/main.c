//
// Created by xp10rd on 3/30/19.
//

#include <stdio.h>
#include "execvpe.h"

extern char **environ;

int main(int argc, char *argv[]) {
    printf("Process environment variables:\n");
    int i;
    for(i = 0; environ[i] != NULL; i++)
        printf("%s\n", environ[i]);

    char *envp[] = {"TEST=VARIABLE", "ALL=WORKS", "ABSOLUTELY=NORMALLY", NULL};
    int e = execvpe(argv[1], argv + 1, envp);
    if(e == -1) {
        perror("error");
    }

    printf("Process environment variables after unsuccessful execvpe:\n");
    for(i = 0; environ[i] != NULL; i++)
        printf("%s\n", environ[i]);

    return -1;
}