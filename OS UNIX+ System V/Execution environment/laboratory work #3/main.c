#include <stdio.h>
#include <unistd.h>//for setuid, get(e)uid

void print_id(FILE* stream) {
    fprintf(stream, "REAL USER ID: %d\n", getuid());
    fprintf(stream,"EFFECTIVE USER ID: %d\n", geteuid());
}

void try_open_file(const char* filename, const char* modes, FILE* stream) {
    FILE* file;
    if(!(file = fopen(filename, modes))) {
        perror("Error");
    }
    else {
        fprintf(stream, "File have been opened successfully!\n");
        fclose(file);
    }
}

int main()
{
    print_id(stderr);
    try_open_file("file", "rw", stderr);
    fprintf(stderr, "--------------------------------------------------------------------------------\n");
    setuid(getuid());
    print_id(stderr);
    try_open_file("file", "rw", stderr);
    return 0;
}
