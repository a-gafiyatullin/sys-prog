#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <errno.h>

void print_error(int status) {
    if(status == -1) {
        perror("Error!");
        exit(-1);
    }
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        return -1;
    }

    int part;
    printf("Task part: ");
    scanf("%d", &part);

    int pid = fork();
    print_error(pid);

    if(part == 1) {
        if (pid == 0) {
            int error = execlp("cat", "cat", argv[1], (char *) 0);  //use PATH
            print_error(error);
        } else {
            printf("Child process has pid: %d\n", pid);
        }
    } else {
        if (pid == 0) {
            int error = execlp("cat", "cat", argv[1], (char *) 0);
            print_error(error);
        } else {
            int child_exit_code;
            int ret_pid = wait(&child_exit_code);
            print_error(ret_pid);
            printf("Child process %d ", pid);
            if(WIFEXITED(child_exit_code)) {
                printf("exit with code %d\n", WEXITSTATUS(child_exit_code));
            } else if(WIFSIGNALED(child_exit_code)) {
                printf("was interrupted with signal %d\n", WTERMSIG(child_exit_code));
            }
        }
    }
    return 0;
}