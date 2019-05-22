#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>

#define NSEMS 5

void test_exit() {
    printf("atexit reaction\n");
}

int main(int argc, char *argv[]) {

    int id, i;
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } semvals;
    struct sembuf semops[NSEMS];

    semvals.array = malloc(sizeof(unsigned short) * NSEMS);
    int flag = (argc > 1 ? SEM_UNDO : 0);
    for(i = 0; i < NSEMS; i++) {
        semvals.array[i] = 1;
        semops[i].sem_num = i;
        semops[i].sem_op = -1;
        semops[i].sem_flg = flag;
    }

    setbuf(stdout, NULL);
    atexit(test_exit);
    printf("Master is creating semaphores...\n");
    if((id = semget(ftok(".", 't'), NSEMS, 0660 | IPC_CREAT)) == -1) {
        perror("Master semget");
        free(semvals.array);
        return -1;
    }
    if(semctl(id, NSEMS,SETALL, semvals) == -1) {
        perror("Master semctl");
        free(semvals.array);
        return -1;
    }
    printf("Master is locking semaphores...\n");
    semop(id, semops, NSEMS);
    printf("Master locked semaphores...\n");
    int c = fgetc(stdin);
    if(c == 't') {
        printf("Master is going to infinity cycle...\n");
        while (1) { }
    } else if(c == 'e') {
        printf("Master is exiting...\n");
        free(semvals.array);
        exit(0);
    } else {
        printf("Master is unlocking semaphores...\n");
        for(i = 0; i < NSEMS; i++) {
            semops[i].sem_op = 1;
        }
        semop(id, semops, NSEMS);
        printf("Master is deleting semaphores...\n");
        semctl(id, NSEMS, IPC_RMID);
    }

    free(semvals.array);
    return 0;
}