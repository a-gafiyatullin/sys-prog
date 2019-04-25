#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>

#define IN 0
#define OUT 1
#define MOD 100
#define LEN 4

int main() {
    srand(time(0));
    FILE *fp[2];
    char num[LEN];

    if(p2open("sort -n", fp) == -1) {
        perror("p2open");
        return -1;
    }

    /* send random data */
    int i;
    for(i = 0; i < MOD; i++) {
        sprintf(num, "%02d\n", rand() % MOD);
        if(fputs(num, fp[IN]) < 0) {
            perror("fputs");
            return -1;
        }
    }
    fclose(fp[IN]);

    /* get sorted data */
    int j;
    for(i = 0; i < MOD / 10; i++) {
        for(j = 0; j < MOD / 10; j++) {
            if(fgets(num, LEN, fp[OUT]) == NULL) {
                perror("fgets");
                return -1;
            }
            num[LEN - 2] = '\0';
            printf("%s ", num);
        }
        putc('\n', stdout);
    }
    fclose(fp[OUT]);

    return 0;
}