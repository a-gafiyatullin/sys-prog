#include <stdio.h>

int main() {
    FILE *child = popen("./server", "w");
    if(child == NULL){
        perror("creating");
        return -1;
    }

    int symbol;
    setbuf(child, NULL);    //without bufferization

    while ((symbol = getc(stdin)) != EOF) {
        putc(symbol, child);
    }

    pclose(child);
    return 0;
}