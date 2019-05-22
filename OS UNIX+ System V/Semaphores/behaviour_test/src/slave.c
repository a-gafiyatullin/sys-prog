#include <stdio.h>
#include <sys/sem.h>

#define NSEMS 5

int main(int argc, char *argv[]) {

    int id, i;
    struct sembuf semops[NSEMS];

    int flag = (argc > 1 ? SEM_UNDO : 0);
    for(i = 0; i < NSEMS; i++) {
        semops[i].sem_num = i;
        semops[i].sem_op = -1;
        semops[i].sem_flg = flag;
    }

    printf("Slave is getting semaphores...\n");
    if((id = semget(ftok(".", 't'), NSEMS, 0)) == -1) {
        perror("child semget");
        return -1;
    }

    printf("Slave is locking semaphores...\n");
    if(semop(id, semops, NSEMS) == -1) {
        perror("semops");
    }
    printf("Slave locked semaphores\n");
    printf("Slave is unlocking semaphores...\n");
    for(i = 0; i < NSEMS; i++) {
        semops[i].sem_op = 1;
    }
    semop(id, semops, NSEMS);

    return 0;
}