#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *in, *out;
    char buffer[BUFSIZ];

    if(argc < 2) {
        in = stdin;
    } else {
        if((in = fopen(argv[1], "r")) == NULL) {
            perror("fopen");
            return -1;
        }
    }

    if((out = popen("wc -l", "w")) == NULL) {
        perror("popen");
        return -1;
    }

    while(fgets(buffer, BUFSIZ, in) != NULL) {
        if(buffer[0] == '\n') {
            fputs(buffer, out);
        }
    }

    fclose(in);
    pclose(out);
    return 0;
}