#include "beep.h"

#include <errno.h>
#include <gpiod.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int beep(const unsigned int gpiochip_num, const size_t pin_num,
         const struct beepUnit sequence[], const size_t sequence_len) {

  struct gpiod_chip *chip;
  struct gpiod_line *line;
  int retval = 0;

  chip = gpiod_chip_open_by_number(gpiochip_num);
  if (!chip) {
    fprintf(stderr, "gpiod_chip_open_by_number(%u) failed\n", gpiochip_num);
    retval = -1;
    goto err_gpiod_chip_open_by_number;
  }

  line = gpiod_chip_get_line(chip, pin_num);
  if (!line) {
    fprintf(stderr, "gpiod_chip_get_line(%lu) failed\n", pin_num);
    goto err_gpiod_chip_get_line;
    retval = -2;
  }

  if (gpiod_line_request_output(line, "beep", 0) != 0) {
    fprintf(stderr, "gpiod_line_request_output() failed\n");
    goto err_gpiod_line_request_output;
    retval = -3;
  }

  for (size_t i = 0; i < sequence_len; ++i) {
    if (gpiod_line_set_value(line, sequence[i].on_off) != 0) {
      fprintf(stderr, "gpiod_line_set_value() error: %d\n", errno);
      break;
    }
    (void)usleep(sequence[i].duration_ms * 1000);
  }

err_gpiod_line_request_output:
  gpiod_line_release(line);
err_gpiod_chip_get_line:
  gpiod_chip_close(chip);
err_gpiod_chip_open_by_number:
  return retval;
}
