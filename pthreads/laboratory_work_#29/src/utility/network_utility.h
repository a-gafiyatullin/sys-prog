#pragma once

#include <fcntl.h>

int set_flag(int fd, int flags); // set flag for socket

int reset_flag(int fd, int flags); // reset flag for socket