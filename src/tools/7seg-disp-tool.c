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
         "    -p, --device-path <device_path>   The path of the GPIO device, typically something like /dev/gpiochip0\n"
         "    -d, --data-pin    <pin_number>    The GPIO pin number in GPIO/BCM schema that connects to the DIO pin\n",
         argv[0]);
  // clang-format on
  _exit(0);
}

void parse_arguments(int argc, char **argv, char **device_path,
                     ssize_t *data_pin) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"device-path", required_argument, 0, 'p'},
        {"data-pin", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "p:d:h", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
    switch (c) {
    case 'p':
      *device_path = optarg;
      break;
    case 'd':
      *data_pin = atoi(optarg);
      break;
    case 'h':
      print_help_then_exit(argv);
      break;
    default:
      print_help_then_exit(argv);
    }
  }
  if (*device_path == NULL || *data_pin <= 0) {
    print_help_then_exit(argv);
  }
}

int main(int argc, char **argv) {
  char *gpio_device_path = NULL;
  ssize_t data_pin = -1;
  parse_arguments(argc, argv, &gpio_device_path, &data_pin);
  struct iotctrl_7seg_display_connection conn = {.display_digit_count = 8,
                                                 .data_pin_num = data_pin,
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
