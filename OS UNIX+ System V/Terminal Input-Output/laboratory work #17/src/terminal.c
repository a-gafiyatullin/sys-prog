#include "terminal.h"

int compare_term_settings(struct termios *term1, struct termios *term2) {
    if(term1->c_lflag != term2->c_lflag) {
        return -1;
    }
    if(term1->c_cc[VTIME] != term2->c_cc[VTIME]) {
        return -1;
    }
    if(term1->c_cc[VMIN] != term2->c_cc[VMIN]) {
        return -1;
    }
    return 0;
}

struct termios *set_non_canonical_mode(int fd, struct termios **old_terminal) {
    if(!isatty(fd)) {
        return NULL;
    }

    /* get current attributes */
    struct termios *old_term = calloc(1, sizeof(struct termios));
    if(tcgetattr(fd, old_term) == -1) {
        return NULL;
    }

    /* save old and set new attributes */
    struct termios *term = calloc(1, sizeof(struct termios));
    *term = *old_term;
    term->c_lflag &= ~(ICANON | ECHO);
    term->c_cc[VMIN] = 1;
    term->c_cc[VTIME] = 0;
    if(tcsetattr(fd, TCSANOW, term) == -1) {
        return NULL;
    }
    /* test if all settings are set */
    struct termios real_term;
    if(tcgetattr(fd, &real_term) == -1) {
        return NULL;
    }
    if(compare_term_settings(term, &real_term) == -1) {
        return NULL;
    }
    *old_terminal = old_term;
    return term;
}

int set_mode(int fd, struct termios *mode) {
    if(!isatty(fd)) {
        return -1;
    }
    if(tcsetattr(fd, TCSANOW, mode) == -1) {
        return -1;
    }
    struct termios real_term;
    if(tcgetattr(fd, &real_term) == -1) {
        return -1;
    }
    if(compare_term_settings(mode, &real_term) == -1) {
        return -1;
    }
    return 0;
}