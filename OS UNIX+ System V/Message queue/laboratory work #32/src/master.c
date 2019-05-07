#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>

#define MSGSIZE 512

int main(int argc, char *argv[]) {

    key_t key;
    int id;
    int alive_recvers_num;
    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    if(argc < 2) {
        return 0;
    }
    alive_recvers_num = atoi(argv[1]);
    fprintf(stdout, "Master started\n");
    if((key = ftok(".", 'g')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = msgget(key, IPC_CREAT | 0600)) == -1) {
        perror("msgget");
        return -1;
    }

    while (msgrcv(id, &msg, MSGSIZE, 0, 0) != -1) {
        if(msg.mtext[0] == 'F' && msg.mtext[1] == '\0') {
            alive_recvers_num--;
            if(alive_recvers_num == 0) {
                break;
            }
            continue;
        }
        fprintf(stdout, "slave: %ld\nstatus: %s\n", msg.mtype, msg.mtext);
    }

    if(msgctl(id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
    } else {
        fprintf(stdout, "Master finished normally\n");
    }

    return 0;
}