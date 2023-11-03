#include "beep.h"

#include <gpiod.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void beep(const size_t signal_pin, struct timespec sequence[], size_t length) {
  struct gpiod_chip *chip;
  struct gpiod_line *line;

  chip = gpiod_chip_open_by_number(0);
  if (!chip) {
    perror("Failed to open GPIO chip");
    return;
  }

  line = gpiod_chip_get_line(chip, signal_pin);
  if (!line) {
    perror("Failed to get GPIO line");
    gpiod_chip_close(chip);
    return;
  }

  gpiod_line_request_output(line, "beeper", 0);

  for (size_t i = 0; i < length; i++) {
    gpiod_line_set_value(line, 1);
    nanosleep(&sequence[i], NULL);
    gpiod_line_set_value(line, 0);
    nanosleep(&sequence[i], NULL);
  }

  gpiod_line_release(line);
  gpiod_chip_close(chip);
}
