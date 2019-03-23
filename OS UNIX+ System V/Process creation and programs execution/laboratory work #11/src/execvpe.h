//
// Created by xp10rd on 3/23/19.
//

#ifndef EXECVPE_H

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from ENVP.  */
int execvpe(const char *file, char *const argv[], char *const envp[]);

#define EXECVPE_H

#endif //EXECVPE_H
