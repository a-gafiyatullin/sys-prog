#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#define MSGSIZE 512

int main() {

    key_t key;
    int id;
    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    fprintf(stdout, "Slave %d started\n", getpid());
    if((key = ftok(".", 'g')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = msgget(key, 0)) == -1) {
        perror("msgget");
        return -1;
    }

    msg.mtype = getpid();
    msg.mtext[0] = 'R';
    msg.mtext[1] = '\0';
    msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
    while (fgets(msg.mtext, MSGSIZE, stdin)) {
        msgsnd(id, &msg, strlen(msg.mtext) + 1, 0);
    }

    msg.mtext[0] = 'F';
    msg.mtext[1] = '\0';
    if(msgsnd(id, &msg, 2, 0) == -1) {
        perror("finishing msgsnd");
    } else {
        fprintf(stdout, "Slave %d finished normally\n", getpid());
    }

    return 0;
}