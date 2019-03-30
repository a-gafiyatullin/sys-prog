//
// Created by xp10rd on 3/30/19.
//

#include "execvpe.h"

extern char **environ;

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    char **environ_bak = environ;
    environ = envp;
    execvp(file, argv); // function will not return if all is ok
    environ = environ_bak;
    return -1;
}