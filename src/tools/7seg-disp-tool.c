#include "iotctrl/7segment-display.h"

#include <gpiod.h>
#include <stdbool.h>
#include <stdio.h>

int main(void) {

  if (iotctrl_init_display("/dev/gpiochip0", 8, 17, 11, 18, 2) != 0)
    return -1;
  iotctrl_update_value(19, 237);
  getchar(); // Unix-based systems
  iotctrl_update_value(145, 9);
  getchar(); // Unix-based systems
  iotctrl_update_value(901, 0);
  getchar(); // Unix-based systems
  iotctrl_update_value(93, 68);
  getchar(); // Unix-based systems
  iotctrl_finalize_7seg_display();
  return 0;
}
