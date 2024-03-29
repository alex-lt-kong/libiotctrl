#include "iotctrl/buzzer.h"

#include <gpiod.h>

#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>

void print_help_then_exit(char **argv) {
  // clang-format off
  printf("Usage: %s\n"
         "    -d, --device-path <device_path>  The path of the GPIO device, typically something like /dev/gpiochip0\n"
         "    -p, --signal-pin  <pin_number>   The GPIO pin number in GPIO/BCM schema\n"
         "    -h, --help                       Print this help message then exit\n",
         argv[0]);
  // clang-format on
  _exit(0);
}

void parse_arguments(int argc, char **argv, char **device_path,
                     ssize_t *pin_num) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"device-path", required_argument, 0, 'd'},
        {"signal-pin", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "d:p:h", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
    switch (c) {
    case 'd':
      *device_path = optarg;
      break;
    case 'p':
      *pin_num = atoi(optarg);
      break;
    default:
      print_help_then_exit(argv);
    }
  }
  if (*device_path == NULL || *pin_num == -1) {
    print_help_then_exit(argv);
  }
}

int main(int argc, char **argv) {
  char *gpio_device_path = NULL;
  ssize_t pin_num = -1;
  parse_arguments(argc, argv, &gpio_device_path, &pin_num);
  const struct iotctrl_buzz_unit sequence[] = {{1, 100}, {0, 100}, {1, 100},
                                               {0, 100}, {1, 100}, {0, 2000}};
  const size_t length = sizeof(sequence) / sizeof(sequence[0]);
  const size_t iter_count = 10;
  printf("Making a test buzz for %zu times using pin %ld...\n", iter_count,
         pin_num);
  for (size_t i = 0; i < iter_count; ++i) {
    printf("Iteration %zu\n", i);
    if (iotctrl_make_a_buzz(gpio_device_path, (size_t)pin_num, sequence,
                            length) != 0) {
      fprintf(stderr, "iotctrl_make_a_buzz() failed\n");
      break;
    }
  }

  return 0;
}
