#include "iotctrl/beep.h"

#include <fcntl.h>
#include <gpiod.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>


#define SIGNAL_PIN 4

int main() {
  int ret;

  struct buzz_unit sequence[] = {{1, 100}, {0, 100}, {1, 100}, {0, 2000},
                                {1, 100}, {0, 100}, {1, 100}, {0, 2000}};
  size_t length = sizeof(sequence) / sizeof(sequence[0]);

  beep(0, 4, sequence, length);


  return 0;
}
