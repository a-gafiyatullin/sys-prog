#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MSGSIZE 512

#define ACCEPT 1

int stop = 0;

void sigint(int signo) { stop = 1; }

int main() {

    key_t key;
    int id, i;
    pid_t sender_pid;
    pid_t my_pid = getpid();

    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    fprintf(stdout, "My pid is: %d\n", my_pid);

    if((key = ftok(".", 'q')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = msgget(key, 0)) == -1) {
        perror("msgget");
        return -1;
    }

    msg.mtype = ACCEPT;
    sprintf(msg.mtext, "%d", my_pid);
    msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
    msgrcv(id, &msg, MSGSIZE, my_pid, 0);
    sender_pid = atoi(msg.mtext);
    if(sender_pid == -1) {
        fprintf(stderr, "Can't connect to recver!\n");
        return -1;
    }

    struct sigaction act;
    act.sa_handler = sigint;
    sigemptyset(&act.sa_mask);
    act.sa_flags &= ~SA_RESTART;
    sigaction(SIGINT, &act, NULL);
    while(!stop) {
        if(msgrcv(id, &msg, MSGSIZE, my_pid, 0) == -1) {
            break;
        }
        if(msg.mtext[0] == 'F' && msg.mtext[1] == '\0') {
            break;
        }
        fprintf(stderr, "%d: %s", my_pid, msg.mtext);
    }

    if(stop) {
        msg.mtype = sender_pid;
        sprintf(msg.mtext, "%d", my_pid);
        if (msgsnd(id, &msg, strlen(msg.mtext) + 1, 0) == -1) {
            perror("msgsnd");
        } else {
            fprintf(stdout, "Recver finished normally\n");
        }
    }

    return 0;
}