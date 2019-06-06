#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <signal.h>

#define MSGSIZE 512

void sigint(int signo) {}

int main(int argc, char *argv[]) {

    key_t key;
    int id;
    int alive_recvers_num = 0;
    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    fprintf(stdout, "Master started\n");
    if((key = ftok(".", 'g')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = msgget(key, IPC_CREAT | 0600)) == -1) {
        perror("msgget");
        return -1;
    }

    signal(SIGINT, sigint);
    while (msgrcv(id, &msg, MSGSIZE, 0, 0) != -1) {
        if(msg.mtext[0] == 'F' && msg.mtext[1] == '\0' && alive_recvers_num != 0) {
            alive_recvers_num--;
            if(alive_recvers_num == 0) {
                break;
            }
            continue;
        } else if(msg.mtext[0] == 'R' && msg.mtext[1] == '\0') {
            alive_recvers_num++;
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