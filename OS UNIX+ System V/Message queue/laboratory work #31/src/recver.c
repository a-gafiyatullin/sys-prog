#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MSGSIZE 512

int id;
pid_t sender_pid;
pid_t my_pid;
struct msgbuf {
    long mtype;
    char mtext[MSGSIZE];
} msg;

void finish_ipc();

void sigint(int signo) { finish_ipc(); }

int main(int argc, char *argv[]) {

    key_t key;
    int i;

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

    signal(SIGINT, sigint);
    while(1) {
        if(msgrcv(id, &msg, MSGSIZE, my_pid, 0) == -1) {
            break;
        }
        if(msg.mtext[0] == 'F' && msg.mtext[1] == '\0') {
            break;
        }
        fprintf(stderr, "%s: %s", argv[0], msg.mtext);
    }

    finish_ipc();
    return 0;
}

void finish_ipc() {
    msg.mtype = sender_pid;
    sprintf(msg.mtext, "%d", my_pid);
    msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
    exit(0);
}