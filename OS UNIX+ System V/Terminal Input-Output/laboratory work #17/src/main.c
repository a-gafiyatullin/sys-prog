#include <fcntl.h>
#include <ctype.h>
#include "terminal.h"

#define LINE_MAX_LENGTH 40
#define BELL '\07'

char backspace_seq[] = {0x8, ' ', 0x8};
void backspace(int fd) { write(fd, backspace_seq, 3); }
void write_symbol(int fd, char ch) { write(fd, &ch, 1); }

int main(int argc, char *argv[]) {
    int fd = open(ctermid(NULL), O_RDWR);
    if(fd == -1) {
        perror("open");
        return -1;
    }

    /* set new terminal settings */
    struct termios *old_term;
    struct termios *term = set_non_canonical_mode(fd, &old_term);
    if(!term) {
        exit(-1);
    }

    /* read and write data */
    char symbol, line[LINE_MAX_LENGTH];
    int len = 0;
    while(read(fd, &symbol, 1) > 0) {
        if(symbol == term->c_cc[VERASE]) {  /* catch backspace key pressing */
            if(len > 0) {
                len--;
                backspace(fd);
            }
            continue;
        }
        if(symbol == term->c_cc[VKILL]) {   /* catch ^U key pressing */
            while (len > 0) {
                len--;
                backspace(fd);
            }
            continue;
        }
        if(symbol == term->c_cc[VWERASE]) { /* catch ^W key pressing */
            while(len > 0 && isspace(line[len - 1])) {
                len--;
                backspace(fd);
            }
            while(len > 0 && !isspace(line[len - 1])) {
                len--;
                backspace(fd);
            }
            continue;
        }
        if(symbol == term->c_cc[VEOF]) {    /* catch ^D key pressing in the beginning of the line */
            if(len == 0) {
                break;
            }
        }
        if(len == LINE_MAX_LENGTH && !isspace(line[len - 1])) {
            while (len > 0 && !isspace(line[len - 1])) {    /* search beginning of the last word */
                len--;
            }
            int i = 0;
            while(len < LINE_MAX_LENGTH) {  /* copy part of the last word to the beginning ot the new line */
                line[i++] = line[len++];
                backspace(fd);
            }
            write_symbol(fd, '\n');
            write(fd, line, i);
            len = i;
        } else if(len == LINE_MAX_LENGTH){
            len = 0;
            write_symbol(fd, '\n');
        }
        if(isprint(symbol)) {
            if(len < LINE_MAX_LENGTH) {
                line[len++] = symbol;
                write(fd, &symbol, 1);
            }
        } else {
            write_symbol(fd, BELL); /* if symbol isn't printed, ring the bell */
        }
    }

    set_mode(fd, old_term); /* restore old terminal settings */
    free(term);
    free(old_term);
    exit(0);
}