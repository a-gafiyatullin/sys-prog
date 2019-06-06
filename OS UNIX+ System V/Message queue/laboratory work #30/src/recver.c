#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>

#define MSGSIZE 512

int main() {
    struct msgbuf {
        long mtype;
        char mtext[MSGSIZE];
    } msg;

    int id = msgget(getuid(), IPC_CREAT | 0660);

    int ret;
    while((ret = msgrcv(id, &msg, MSGSIZE, 0, MSG_NOERROR)) != -1) {
        msg.mtext[ret] = '\0';
        fprintf(stdout, "Message contains: %s", msg.mtext);
        fprintf(stdout, "Message type: %ld\n", msg.mtype);
        fprintf(stdout, "msgrcv return code: %d\n\n", ret);
    }

    return 0;
}