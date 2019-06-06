#ifndef TERMINAL_H
#define TERMINAL_H

#include <termio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct termios *set_non_canonical_mode(int fd, struct termios **old_term);

int set_mode(int fd, struct termios *mode);

#endif /* TERMINAL_H */
