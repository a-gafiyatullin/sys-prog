#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

int main() {
    char answer;
    struct termios term, term_bak, real_term;
    /* set non-canonical mode */
    int fd = open("/dev/tty", O_RDWR);
    if(tcgetattr(fd, &term) == -1) {
        perror("error");
        close(fd);
        return errno;
    }
    term_bak = term;
    term.c_lflag &= ~ICANON;
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    if(tcsetattr(fd, TCSANOW, &term) == -1) {
        perror("error");
        close(fd);
        return errno;
    }
    if(tcgetattr(fd, &real_term) == -1) {
        perror("error");
        close(fd);
        return errno;
    }
    /* test if settings are really changed */
    if(real_term.c_lflag != term.c_lflag || real_term.c_cc[VMIN] != term.c_cc[VMIN]
        || real_term.c_cc[VTIME] != term.c_cc[VTIME]) {
        close(fd);
        return EAGAIN;
    }
    /* get answer */
    printf("Is UNIX the best OS in the world? [Y/N]:\n");
    answer = getc(stdin);
    if(answer == 'Y' || answer == 'y') {
        printf("\nYou are right!\n");
    } else if(answer == 'N' || answer == 'n'){
        printf("\nHmm, i think, you should change your mind!\n");
    } else {
        printf("\nNo such answer!\n");
    }

    /* restore old settings */
    tcsetattr(fd, TCSANOW, &term_bak);
    close(fd);
    return 0;
}