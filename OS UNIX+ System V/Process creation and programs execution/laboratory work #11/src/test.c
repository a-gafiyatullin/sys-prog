#include <stdio.h>
extern char **environ;

int main(int argc, char *argv[], char *envp[]) {
    printf("Process environment variables after successful execvpe:\n");
    int i;
    for(i = 0; envp[i] != NULL; i++)
        printf("%s\n", envp[i]);

    printf("Process arguments after successful execvpe:\n");
    for(i = 0; i < argc; i++)
        printf("%s\n", argv[i]);

    return 0;
}