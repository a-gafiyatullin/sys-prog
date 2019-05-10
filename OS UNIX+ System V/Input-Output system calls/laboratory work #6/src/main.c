#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define STRINGS_MAX_NUM 100
#define TIME_OUT 5

int file; //variables in a global scope because of sig_alarm
off_t string_max_length;
char *string;


int create_search_table(off_t *indents, off_t *strings_length);
void print_file();

void sig_alarm(int signo){ }

int main(int argc, char *argv[]) {
    /* open file */
    if(argc != 2) {
        fprintf(stderr, "should be only one argument!\n");
        return -1;
    }

    file = open(argv[1], O_RDONLY);
    if(file == -1) {
        perror("open:");
        return -1;
    }

    /* get tables */
    off_t indents[STRINGS_MAX_NUM];
    off_t strings_length[STRINGS_MAX_NUM];
    int strings_num;
    if((strings_num = create_search_table(indents, strings_length)) == -1) {
        exit(-1);
    }

    /* change SIGALRM disposition */
    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sig.sa_handler = sig_alarm;
    sig.sa_flags = 0;
    if(sigaction(SIGALRM, &sig, NULL) == -1) {
        perror("sigaction");
        exit(-1);
    }

    /* answer user's requests */
    int num, len;
    string = (char*)malloc(sizeof(char) * string_max_length);
    while (1) {
        alarm(TIME_OUT);
        printf("Input string number: ");
        if(scanf("%d", &num) != 1) {
            if(errno == EINTR) {
                print_file();
            }
            break;
        }
        alarm(0);
        if(num == 0) {
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

int create_search_table(off_t *indents, off_t *strings_length) {
    char symbol;
    off_t string_length = 0, file_length = 0;
    int i = 0;
    string_max_length = 0;
    while (read(file, &symbol, sizeof(char)) > 0 && i < STRINGS_MAX_NUM) {
        file_length++;
        string_length++;
        if(symbol == '\n') {
            indents[i] = file_length;
            strings_length[i] = string_length;
            if(string_length > string_max_length) {
                string_max_length = string_length;
            }
            string_length = 0;
            i++;
        }
    }

    if(i < STRINGS_MAX_NUM) {
        indents[i] = file_length;
        strings_length[i] = string_length;
        if (string_length > string_max_length) {
            string_max_length = string_length;
        }
    }

    if(lseek(file, 0, SEEK_SET) == -1) {
        perror("lseek");
        return -1;
    }
    return i;
}

void print_file() {
    int len;
    if(lseek(file, 0, SEEK_SET) == -1) {
        return;
    }
    while ((len = read(file, string, sizeof(char) * string_max_length)) > 0) {
        if(write(STDOUT_FILENO, string, sizeof(char) * len) <= 0) {
            break;
        }
    }
}