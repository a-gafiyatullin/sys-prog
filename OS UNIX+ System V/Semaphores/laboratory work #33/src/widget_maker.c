#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <signal.h>

#define NSEMS 3

int id;

void sigint(int signo) {
    semctl(id, NSEMS, IPC_RMID);
    exit(0);
}

int main() {

    int key;
    int i;
    union semun {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } semvals;
    struct sembuf op[NSEMS];

    if((key = ftok(".", 'm')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = semget(key, NSEMS, 0660 | IPC_CREAT)) == -1) {
        perror("semget");
        return -1;
    }
    if((semvals.array = malloc(sizeof(ushort) * NSEMS)) == NULL) {
        return -1;
    }
    for(i = 0; i < NSEMS; i++) {
        op[i].sem_op = -1;
        op[i].sem_num = i;
        op[i].sem_flg = 0;
        semvals.array[i] = 0;
    }
    if(semctl(id, NSEMS, SETALL, semvals) == -1) {
        perror("semctl");
        return -1;
    }

    signal(SIGINT, sigint);
    while(1) {
        if(semop(id, op, NSEMS) != -1) {
            printf("Add a widget\n");
        }
    }
}