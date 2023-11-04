#include "iotctrl/beep.h"

#include <fcntl.h>
#include <gpiod.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>

#define LOCK_FILE_PATH "/tmp/beeper.lck"
#define SIGNAL_PIN 4

int main() {
  int ret, fd;

  fd = open(LOCK_FILE_PATH, O_RDWR | O_CREAT, 0644);
  if (fd < 0) {
    perror("Failed to open lock file");
    exit(1);
  }

  ret = flock(fd, LOCK_EX | LOCK_NB);
  if (ret != 0) {
    printf("File already locked\n");
    return -1;
  }

  printf("File available, not locked\n");
  /*struct timespec sequence[] = {{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
                                {1, 0}, {1, 0}, {1, 0}, {1, 0}};*/
  struct beepUnit sequence[] = {{1, 100}, {0, 100}, {1, 100}, {0, 2000},
                                {1, 100}, {0, 100}, {1, 100}, {0, 2000}};
  size_t length = sizeof(sequence) / sizeof(sequence[0]);

  beep(0, 4, sequence, length);
  flock(fd, LOCK_UN);
  close(fd);

  return 0;
}
