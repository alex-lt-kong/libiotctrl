#include "iotctrl/7segment-display.h"

#include <gpiod.h>

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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
  struct iotctrl_7seg_display_connection conn = {.display_digit_count = 8,
                                                 .data_pin_num = 17,
                                                 .clock_pin_num = 11,
                                                 .latch_pin_num = 18,
                                                 .chain_num = 2};
  if (iotctrl_init_display(gpio_device_path, conn) != 0) {
    fprintf(stderr, "iotctrl_init_display() failed: %d(%s)\n", errno,
            strerror(errno));
    return -1;
  }
  const float values[][2] = {{19, 237.4}, {-145, 9},       {901, -0},
                             {-3.2, 68},  {-99.99, 3.141}, {-73.9, 734},
                             {-0.1, 0.6}, {99.8, -1234}};
  const size_t len = sizeof(values) / sizeof(values[0]);
  for (size_t i = 0; i < len; ++i) {
    float val0 = values[i][0];
    val0 = val0 > 999.9 ? 0 : val0;
    val0 = val0 < -99.9 ? 0 : val0;
    float val1 = values[i][1];
    val1 = val1 > 999.9 ? 0 : val1;
    val1 = val1 < -99.9 ? 0 : val1;
    printf("Now showing: %f\t%f\n", val0, val1);
    iotctrl_update_value_two_four_digit_floats(val0, val1);
    getchar();
  }
  return 0;
}
