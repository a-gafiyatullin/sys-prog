#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define BEL '\07'

static void beep(int);

int main() {
    signal(SIGINT, beep);
    signal(SIGQUIT, beep);
    for(;;)
        pause();
}


static void beep(int signo) {
    static int counter = 0;
    if(signo == SIGINT) {
        printf("%c\n", BEL);
        counter++;
    } else {
        printf("Beep counter: %d\n", counter);
        exit(0);
    }
    signal(SIGINT, beep);
}