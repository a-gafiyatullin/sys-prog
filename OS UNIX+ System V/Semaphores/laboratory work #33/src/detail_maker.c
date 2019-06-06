#include <stdio.h>
#include <sys/sem.h>
#include <unistd.h>

#define NSEMS 3
#define DETAIL_A_MAKER 0
#define DETAIL_B_MAKER 1
#define DETAIL_C_MAKER 2

#define DETAIL_A_TIME 2
#define DETAIL_B_TIME 3
#define DETAIL_C_TIME 4

int main(int argc, char *argv[]) {

    int id, key;
    int maker_type, time_delay;
    struct sembuf op;
    char *message[NSEMS] = {
            "Add a detail 'A'\n",
            "Add a detail 'B'\n",
            "Add a detail 'C'\n"
    };

    if(argc < 2) {
        fprintf(stderr, "Detail maker must have type!\n");
        return -1;
    }
    switch (argv[1][0]) {
        case 'A':
            maker_type = DETAIL_A_MAKER;
            time_delay = DETAIL_A_TIME;
            break;
        case 'B':
            maker_type = DETAIL_B_MAKER;
            time_delay = DETAIL_B_TIME;
            break;
        case 'C':
            maker_type = DETAIL_C_MAKER;
            time_delay = DETAIL_C_TIME;
            break;
        default:
            fprintf(stderr, "Unrecognized type!\n");
            return -1;
    }

    if((key = ftok(".", 'm')) == -1) {
        perror("ftok");
        return -1;
    }
    if((id = semget(key, NSEMS, 0)) == -1) {
        perror("semget");
        return -1;
    }
    op.sem_op = 1;
    op.sem_num = maker_type;
    op.sem_flg = 0;

    while (1) {
        sleep(time_delay);
        if(semop(id, &op, 1) != -1) {
            printf("%s", message[maker_type]);
        } else {
            break;
        }
    }

    return 0;
}