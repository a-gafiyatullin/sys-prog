#include "network_utility.h"

int set_flag(int fd, int flags) {
  int val;
  if ((val = fcntl(fd, F_GETFL, 0)) < 0) {
    return -1;
  }
  val |= flags;
  if (fcntl(fd, F_SETFL, val) < 0) {
    return -1;
  }

  return 0;
}

int reset_flag(int fd, int flags) {
  int val;
  if ((val = fcntl(fd, F_GETFL, 0)) < 0) {
    return -1;
  }
  val &= (~flags);
  if (fcntl(fd, F_SETFL, val) < 0) {
    return -1;
  }

  return 0;
}