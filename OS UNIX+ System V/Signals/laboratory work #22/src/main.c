#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <termio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define TIME_OUT 5 //time in seconds
#define STRINGSIZ 256

int open_files(int *files, int num, char **paths);

int multiplexed_read(int *files, int num);

int main(int argc, char *argv[]) {
    if(argc < 2) {
        return EINVAL;
    }
    int *files = (int*)malloc(sizeof(int) * MIN((argc - 1), _POSIX_OPEN_MAX));
    if(open_files(files, argc - 1, argv + 1) == -1) {
        return errno;
    }

    if(multiplexed_read(files, argc - 1) == -1) {
        return errno;
    }
    free(files);
    return 0;
}

int open_files(int *files, int num, char **paths) {
    int i;
    for(i = 0; i < num; i++) {
        if((files[i] = open(paths[i], O_RDONLY)) == -1) {
            perror("Can't open the file");
            return -1;
        }
    }
    return 0;
}

void sig_alarm(int signum) { }

int multiplexed_read(int *files, int num) {
    /* set signal handler */
    struct sigaction act;
    act.sa_handler = sig_alarm;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_INTERRUPT;
    sigaction(SIGALRM, &act, NULL);
    /* main part of the function */
    char buffer[STRINGSIZ];
    int current_file = -1, opened_num = num, length;
    while(opened_num) {
        if(++current_file == num) {
            current_file = 0;
        }
        if(files[current_file] == -1) {
            continue;
        }
        alarm(TIME_OUT);    //set alarm
        if((length = read(files[current_file], buffer, STRINGSIZ)) == -1) {
            alarm(0);   //stop alarm
            if(errno == EINTR) {
                continue;
            } else {
                close(files[current_file]);
                files[current_file] = -1;
                opened_num--;
            }
        } else {
            alarm(0);   //stop alarm
            //write to stdout
            if(write(1, buffer, length) == -1) {
                perror("Can't write to stdout");
                return -1;
            }
        }
    }
    return 0;
}