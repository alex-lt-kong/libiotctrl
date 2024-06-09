#include "iotctrl/7segment-display.h"

#include <gpiod.h>

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t ev_flag = 0;

static void signal_handler(int signum) {
  char msg[] = "Signal [  ] caught\n";
  msg[8] = '0' + (char)(signum / 10);
  msg[9] = '0' + (char)(signum % 10);
  write(STDIN_FILENO, msg, strlen(msg));
  ev_flag = 1;
}

static int install_signal_handler() {
  // This design canNOT handle more than 99 signal types
  if (_NSIG > 99) {
    fprintf(stderr, "signal_handler() can't handle more than 99 signals");
    return -1;
  }
  struct sigaction act;
  // Initialize the signal set to empty, similar to memset(0)
  if (sigemptyset(&act.sa_mask) == -1) {
    fprintf(stderr, "sigemptyset(): %d(%s)", errno, strerror(errno));
    return -1;
  }
  act.sa_handler = signal_handler;
  /* SA_RESETHAND means we want our signal_handler() to intercept the
signal once. If a signal is sent twice, the default signal handler will be
used again. `man sigaction` describes more possible sa_flags. */
  act.sa_flags = SA_RESETHAND;
  // act.sa_flags = 0;
  if (sigaction(SIGINT, &act, 0) == -1 || sigaction(SIGABRT, &act, 0) == -1 ||
      sigaction(SIGTERM, &act, 0) == -1) {
    fprintf(stderr, "sigaction(): %d(%s)", errno, strerror(errno));
    return -1;
  }
  return 0;
}

void print_help_then_exit(char **argv) {
  // clang-format off
  printf("Usage: %s\n"
         "    -p, --device-path <device_path> The path of the GPIO device, typically something like /dev/gpiochip0\n"
         "    -d, --data-pin    <pin_number>  The GPIO pin number in GPIO/BCM schema that connects to the DIO pin\n"
         "    -n, --digit-count <count>       Number of digit of a digital tube, it must be either four or eight\n",
         argv[0]);
  // clang-format on
  _exit(0);
}

void parse_arguments(int argc, char **argv, char **device_path,
                     ssize_t *data_pin, ssize_t *digit_count) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"device-path", required_argument, 0, 'p'},
        {"data-pin", required_argument, 0, 'd'},
        {"digit-count", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "p:d:n:h", long_options, &option_index);

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
    case 'n':
      *digit_count = atoi(optarg);
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
  int retval = 0;
  ssize_t data_pin = -1, digit_count = -1;
  char *gpio_device_path = NULL;
  if (install_signal_handler() != 0) {
    retval = -1;
    goto err_signal_handler_install;
  }
  struct iotctrl_7seg_disp_connection conn;
  parse_arguments(argc, argv, &gpio_device_path, &data_pin, &digit_count);
  conn.display_digit_count = digit_count;
  conn.data_pin_num = data_pin;
  conn.clock_pin_num = 11;
  conn.latch_pin_num = 18;
  conn.chain_num = 2;
  conn.refresh_rate_hz = 40 * 1000;
  strcpy(conn.gpiochip_path, gpio_device_path);

  printf("Parameters:\n");
  printf("display_digit_count: %zu\n", conn.display_digit_count);
  printf("data_pin_num: %d\n", conn.data_pin_num);
  printf("clock_pin_num: %d\n", conn.clock_pin_num);
  printf("latch_pin_num: %d\n", conn.latch_pin_num);
  printf("chain_num: %d\n", conn.chain_num);
  printf("gpiochip_path: %s\n", conn.gpiochip_path);

  struct iotctrl_7seg_disp_handle *handle;

  if ((handle = iotctrl_7seg_disp_init(conn)) == NULL) {
    fprintf(stderr, "iotctrl_7seg_disp_init() failed: %d(%s)\n", errno,
            strerror(errno));
    return -1;
  }
  const float values[][2] = {{-99.9, -99.9}, {-88.8, -88.8}, {-77.7, -77.7},
                             {-66.6, -66.6}, {-55.5, -55.5}, {-44.4, -44.4},
                             {-33.3, -33.3}, {-22.2, -22.2}, {-11.1, -11.1},
                             {111.1, 111.1}, {222.2, 222.2}, {333.3, 333.3},
                             {444.4, 444.4}, {555.5, 555.5}, {666.6, 666.6},
                             {777.7, 777.7}, {888.8, 888.8}, {999.9, 999.9}};
  const size_t len = sizeof(values) / sizeof(values[0]);
  while (!ev_flag) {
    for (size_t i = 0; i < len && !ev_flag; ++i) {
      if (digit_count == 8) {
        printf("Now showing: %.1f, %.1f\n", values[i][0], values[i][1]);
        iotctrl_7seg_disp_update_as_four_digit_float(*handle, values[i][0], 0);
        iotctrl_7seg_disp_update_as_four_digit_float(*handle, values[i][1], 1);
      } else {
        printf("Now showing: %.1f\n", values[i][0]);
        iotctrl_7seg_disp_update_as_four_digit_float(*handle, values[i][0], 0);
      }
      sleep(1);
    }
  }
  printf("Done\n");
  iotctrl_7seg_disp_destory(handle);
err_signal_handler_install:
  return retval;
}
