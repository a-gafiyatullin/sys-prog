#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MSGSIZE 512
#define MAX_SENDRERS_NUM 255

#define ACCEPT 1

#define TRUE 1
#define FALSE 0

int set_non_blocking_input();

int main() {

    key_t key;
    int id, i;
    int senders_num = 0;
    int alive_recvers_num = senders_num;
    pid_t  pid = getpid(), recv_pid;

    struct recver_info {
        pid_t pid;
        int is_alive;
    } recvers[MAX_SENDRERS_NUM];

    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    if((key = ftok(".", 'q')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = msgget(key, IPC_CREAT | 0600)) == -1) {
        perror("msgget");
        return -1;
    }

    if(set_non_blocking_input() == -1) {
        return -1;
    }

    for (i = 0; i < MAX_SENDRERS_NUM; ++i) {
        recvers[i].is_alive = FALSE;
    }
    while(!feof(stdin)) {
        if(msgrcv(id, &msg, MSGSIZE, ACCEPT, IPC_NOWAIT) != -1) {
            msg.mtype = atoi(msg.mtext);
            i = 0;
            while (i < MAX_SENDRERS_NUM && recvers[i].is_alive) {
                i++;
            }
            if(i != MAX_SENDRERS_NUM) {
                recvers[i].pid = msg.mtype;
                recvers[i].is_alive = TRUE;
                sprintf(msg.mtext, "%d", pid);
                ++alive_recvers_num;
            } else {
                sprintf(msg.mtext, "-1");
            }
            senders_num = i + 1;
            msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
        }
        if(msgrcv(id, &msg, MSGSIZE, pid, IPC_NOWAIT) != -1) {
            recv_pid = atoi(msg.mtext);
            for(i = 0; i < senders_num; i++) {
                if(recvers[i].pid == recv_pid) {
                    recvers[i].is_alive = FALSE;
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
        for(i = 0; i < senders_num; i++) {
            if(recvers[i].is_alive) {
                msg.mtype = recvers[i].pid;
                msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
            }
        }
    }

    if(alive_recvers_num != 0) {
        msg.mtext[0] = 'F';
        msg.mtext[1] = '\0';
        for (i = 0; i < senders_num; i++) {
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