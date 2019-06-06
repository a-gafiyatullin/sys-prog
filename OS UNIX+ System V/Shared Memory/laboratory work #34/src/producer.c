#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BUFSIZE 128

#define SEM_READ 0
#define SEM_WRITE 1

int main() {

    key_t key;
    int semid, shmid;
    char *product;
    ushort sem_vals[2] = {0 , 1};
    struct sembuf sem_op[2];

    sem_op[SEM_READ].sem_num = SEM_READ;
    sem_op[SEM_READ].sem_op = 1;
    sem_op[SEM_READ].sem_flg = SEM_UNDO;

    sem_op[SEM_WRITE].sem_num = SEM_WRITE;
    sem_op[SEM_WRITE].sem_op = -1;
    sem_op[SEM_WRITE].sem_flg = SEM_UNDO;

    if((key = ftok(".", 'm')) < 0) {
        perror("ftok");
        return -1;
    }
    if((shmid = shmget(key, sizeof(char) * BUFSIZE, IPC_CREAT | 0660)) < 0) {
        perror("shmget");
        return -1;
    }
    if((product = shmat(shmid, NULL, 0)) == NULL) {
        perror("shmat");
        return -1;
    }

    if((key = ftok(".", 's')) < 0) {
        perror("ftok");
        return -1;
    }
    if((semid = semget(key, 2, IPC_CREAT | 0660)) < 0) {
        perror("semget");
        return -1;
    }
    if(semctl(semid, 2, SETALL, sem_vals) < 0) {
        perror("semctl");
        return -1;
    }

    while(1) {
        if(semop(semid, &sem_op[SEM_WRITE], 1) < 0) {
            break;
        }
        if(fgets(product, BUFSIZE, stdin) == NULL) {
            break;
        }
        if(semop(semid, &sem_op[SEM_READ], 1) < 0) {
            break;
        }
    }

    if(shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl");
    }
    shmdt(product);
    if(semctl(semid, 2, IPC_RMID) < 0) {
        perror("semctl");
    }
    return 0;
}