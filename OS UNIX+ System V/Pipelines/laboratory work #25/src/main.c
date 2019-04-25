#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <wait.h>

#define IN 0
#define OUT 1

int main() {
    int pipeline[2];
    pipe(pipeline);
    char symbol;
    if(fork() == 0) {
        printf("I'm first child process!\n");
        close(pipeline[IN]);
        while((symbol = getc(stdin)) != EOF) {
            write(pipeline[OUT], &symbol, 1);
        } close(pipeline[OUT]);
    } else {
        if(fork() == 0) {
            printf("I'm second child process!\n");
            close(pipeline[OUT]);
            while(read(pipeline[IN], &symbol, 1) != -1) {
                putc(toupper(symbol), stdout);
            }
            close(pipeline[IN]);
        } else {
            close(pipeline[IN]);    //parent code
            close(pipeline[OUT]);
            int stat;
            while(wait(&stat) != -1) { }
        }
    }
    return 0;
}