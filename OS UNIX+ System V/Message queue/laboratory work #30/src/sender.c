#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MSGSIZE 512

int main(int argc, char *argv[]) {
    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    int id = msgget(argc < 2 ? getuid() : atoi(argv[1]), 0);

    while(1) {
        fscanf(stdin, "%ld", &msg.mtype);
        if(msg.mtype == -1) {
            break;
        }
        fgets(msg.mtext, BUFSIZ, stdin);
        msgsnd(id, &msg, strlen(msg.mtext), 0);
    }

    msgctl(id, IPC_RMID, NULL);
    return 0;
}