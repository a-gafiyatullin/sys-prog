#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define BELL '\07'

void beep(int);

int main() {
    struct sigaction act;
    act.sa_handler = beep;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_INTERRUPT;
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    for(;;)
        pause();
}


void beep(int signo) {
    static int counter = 0;
    if(signo == SIGINT) {
        printf("%c\n", BELL);
        counter++;
    } else {
        printf("Beep counter: %d\n", counter);
        exit(0);
    }
}