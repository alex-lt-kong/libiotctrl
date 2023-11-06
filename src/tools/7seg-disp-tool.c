#include "iotctrl/7segment-display.h"

#include <gpiod.h>

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

void print_help_then_exit(char **argv) {
  // clang-format off
  printf("Usage: %s\n"
         "    -d, --device-path <device_path>   The path of the GPIO device, typically something like /dev/gpiochip0\n",
         argv[0]);
  // clang-format on
  _exit(0);
}

void parse_arguments(int argc, char **argv, char **device_path) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"device-path", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "d:h", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
    switch (c) {
    case 'd':
      *device_path = optarg;
      break;
    case 'h':
      print_help_then_exit(argv);
      break;
    default:
      print_help_then_exit(argv);
    }
  }
  if (*device_path == NULL) {
    print_help_then_exit(argv);
  }
}

int main(int argc, char **argv) {
  char *gpio_device_path = NULL;
  parse_arguments(argc, argv, &gpio_device_path);
  struct iotctrl_7seg_display_connection conn = {8, 17, 11, 18, 2};
  if (iotctrl_init_display(gpio_device_path, conn) != 0)
    return -1;
  iotctrl_update_value_two_four_digit_floats(19, 237.4);
  getchar(); // Unix-based systems
  iotctrl_update_value_two_four_digit_floats(-145, 9);
  getchar(); // Unix-based systems
  iotctrl_update_value_two_four_digit_floats(901, -0);
  getchar(); // Unix-based systems
  iotctrl_update_value_two_four_digit_floats(-3.2, 68);
  getchar(); // Unix-based systems
  iotctrl_update_value_two_four_digit_floats(-99.99, 3.141);
  getchar(); // Unix-based systems
  iotctrl_update_value_two_four_digit_floats(-73.9, 734);
  getchar(); // Unix-based systems
  iotctrl_update_value_two_four_digit_floats(-0.1, 0.6);
  getchar(); // Unix-based systems
  iotctrl_finalize_7seg_display();
  return 0;
}
