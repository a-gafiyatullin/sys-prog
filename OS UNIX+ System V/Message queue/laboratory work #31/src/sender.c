#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MSGSIZE 512

int set_non_blocking_input();

int main(int argc, char *argv[]) {

    key_t key;
    int id;
    int recv_pid;
    int alive_recvers_num = argc - 1;
    pid_t pid = getpid();
    struct recver_info {
        pid_t pid;
        int is_alive;
    } *recvers;
    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;
    int i;

    fprintf(stdout, "My pid is: %d\n", pid);
    if((key = ftok(".", 'q')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = msgget(key, IPC_CREAT | 0600)) == -1) {
        perror("msgget");
        return -1;
    }
    if((recvers = malloc(sizeof(struct recver_info) * (argc - 1))) == NULL) {
        perror("malloc");
        return -1;
    }
    for(i = 0; i < argc - 1; i++) {
        recvers[i].pid = atoi(argv[i + 1]);
        recvers[i].is_alive = 1;
    }

    if(set_non_blocking_input() == -1) {
        return -1;
    }
    while(!feof(stdin)) {
        if(msgrcv(id, &msg, MSGSIZE, pid, IPC_NOWAIT) != -1) {
            recv_pid = atoi(msg.mtext);
            for(i = 0; i < argc - 1; i++) {
                if(recvers[i].pid == recv_pid) {
                    recvers[i].is_alive = 0;
                    break;
                }
            }
            alive_recvers_num--;
            if(alive_recvers_num == 0) {
                break;
            }
        }
        if(fgets(msg.mtext, MSGSIZE, stdin) == NULL) {
            continue;
        }
        for(i = 0; i < argc - 1; i++) {
            if(recvers[i].is_alive) {
                msg.mtype = recvers[i].pid;
                msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
            }
        }
    }

    if(alive_recvers_num != 0) {
        msg.mtext[0] = 'F';
        msg.mtext[1] = '\0';
        for (i = 0; i < argc - 1; i++) {
            if(recvers[i].is_alive) {
                msg.mtype = recvers[i].pid;
            }
            msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
        }
    }
    if(msgctl(id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
    } else {
        fprintf(stdout, "Sender finished normally\n");
    }
    free(recvers);
    return 0;
}

int set_non_blocking_input() {

    int iflags;

    if((iflags = fcntl(STDIN_FILENO, F_GETFL, 0)) == -1) {
        perror("fcntl");
        return -1;
    }
    iflags |= O_NONBLOCK;
    if(fcntl(STDIN_FILENO, F_SETFL, iflags) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}