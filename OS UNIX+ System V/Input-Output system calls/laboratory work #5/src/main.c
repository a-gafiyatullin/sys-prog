#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define STRINGS_MAX_NUM 100

int create_search_table(int fd, off_t *indents, off_t *strings_length, off_t *string_max_length);

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "should be only one argument!\n");
        return -1;
    }

    int file = open(argv[1], O_RDONLY);
    if(file == -1) {
        perror("open");
        return -1;
    }
    off_t indents[STRINGS_MAX_NUM];
    off_t strings_length[STRINGS_MAX_NUM];
    off_t string_max_length;
    int strings_num;
    if((strings_num = create_search_table(file, indents, strings_length, &string_max_length)) == -1) {
        exit(-1);
    }

    int num, len;
    char *string = (char*)malloc(sizeof(char) * string_max_length);
    while (1) {
        printf("Input string number: ");
        if(scanf("%d", &num) != 1 || num == 0) {
            break;
        }
        if(num > strings_num || num < 0) {
            fprintf(stderr, "Unexpected string number!\n");
            continue;
        }
        if(lseek(file, indents[num - 1] - strings_length[num - 1], SEEK_SET) == -1) {
            perror("lseek");
            break;
        }
        len = read(file, string, strings_length[num - 1]);
        if(len <= 0 || write(STDOUT_FILENO, string, sizeof(char) * len) <= 0) {
            break;
        }
    }

    free(string);
    exit(0);
}

int create_search_table(int fd, off_t *indents, off_t *strings_length, off_t *string_max_length) {
    char symbol;
    off_t string_length = 0, file_length = 0;
    int i = 0;
    *string_max_length = 0;
    while (read(fd, &symbol, sizeof(char)) && i < STRINGS_MAX_NUM) {
        file_length++;
        string_length++;
        if(symbol == '\n') {
            indents[i] = file_length;
            strings_length[i] = string_length;
            if(string_length > *string_max_length) {
                *string_max_length = string_length;
            }
            string_length = 0;
            i++;
        }
    }

    if(i < STRINGS_MAX_NUM) {
        indents[i] = file_length;
        strings_length[i] = string_length;
        if (string_length > *string_max_length) {
            *string_max_length = string_length;
        }
    }

    if(lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        return -1;
    }
    return i;
}