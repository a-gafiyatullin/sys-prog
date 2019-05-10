#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MSGSIZE 512

int stop = 0;

void sigint(int signo) { stop = 1; }

int main(int argc, char *argv[]) {

    key_t key;
    int i;
    int id;
    pid_t sender_pid;
    pid_t my_pid;
    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    my_pid = getpid();
    fprintf(stdout, "My name is %s and my pid is: %d\n", argv[0], my_pid);
    fprintf(stdout, "Sender pid: ");
    if(fscanf(stdin, "%d", &sender_pid) != 1) {
        perror("fscanf");
        return -1;
    }
    if((key = ftok(".", 'q')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = msgget(key, 0)) == -1) {
        perror("msgget");
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
        fprintf(stderr, "%s: %s", argv[0], msg.mtext);
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