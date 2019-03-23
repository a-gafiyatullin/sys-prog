#include <stdio.h>
extern char **environ;
int main() {
    printf("Process environment variables after successful execvpe:\n");
    for(int i = 0; environ[i] != NULL; i++)
        printf("%s\n", environ[i]);

    return 0;
}