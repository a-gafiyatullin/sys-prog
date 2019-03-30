//
// Created by xp10rd on 3/23/19.
//

#include "execvpe.h"

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    if (*file == '\0') {
        errno = ENOENT;
        return -1;
    }
    if (strchr (file, '/') != NULL) {
        /* Don't search when it contains a slash.  */
        execve(file, argv, envp);
    } else {
        size_t len, pathlen;
        char *name, *p;
        char *path = getenv("PATH");
        if (path == NULL)
            path = ":/bin:/usr/bin";

        len = strlen(file) + 1;
        pathlen = strlen(path);
        char *buf = malloc(sizeof(char) * (len + pathlen));
        name = memcpy(buf + pathlen + 1, file, len);
        /* And add the slash.  */
        *--name = '/';

        p = path;
        do {
            char *startp;

            path = p;
            p = strchr(path, ':');
            if (!p)
                p = strchr(path, '\0');

            if (p == path)
                /* Two adjacent colons, or a colon at the beginning or the end
                   of `PATH' means to search the current directory.  */
                startp = name + 1;
            else
                startp = memcpy(name - (p - path), path, p - path);

            /* Try to execute this name.  If it works, execv will not return.  */
            execve(startp, argv, envp);

            switch (errno) {
                case EACCES:
                    /* Record the we got a `Permission denied' error.  If we end
                       up finding no executable we can use, we want to diagnose
                       that we did find one but were denied access.  */
                case ENOENT:
                case ESTALE:
                case ENOTDIR:
                    /* Those errors indicate the file is missing or not executable
                       by us, in which case we want to just try the next path
                       directory.  */
                case ENODEV:
                case ETIMEDOUT:
                    /* Some strange filesystems like AFS return even
                       stranger error numbers.  They cannot reasonably mean
                       anything else so ignore those, too.  */
                case ENOEXEC:
                    /* We won't go searching for the shell
                     * if it is not executable - the Linux
                     * kernel already handles this enough,
                     * for us. */
                    break;

                default:
                    /* Some other error means we found an executable file, but
                       something went wrong executing it; return the error to our
                       caller.  */
                    return -1;
            }
        } while (*p++ != '\0');

        free(buf);
    }

    /* Return, errno contains the error from the last attempt (probably ENOENT).  */
    return -1;
}

