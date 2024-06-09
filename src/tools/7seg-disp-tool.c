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
                     ssize_t *data_pin, ssize_t *chain_count) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"device-path", required_argument, 0, 'p'},
        {"data-pin", required_argument, 0, 'd'},
        {"chain-count", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "p:d:c:h", long_options, &option_index);

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
    case 'c':
      *chain_count = atoi(optarg);
      break;
    case 'h':
      print_help_then_exit(argv);
      break;
    default:
      print_help_then_exit(argv);
    }
  }
  if (*device_path == NULL || *data_pin <= 0 || *chain_count < 0) {
    print_help_then_exit(argv);
  }
}

int main(int argc, char **argv) {
  int retval = 0;
  ssize_t data_pin = -1, chain_count = -1;
  char *gpio_device_path = NULL;
  if (install_signal_handler() != 0) {
    retval = -1;
    goto err_signal_handler_install;
  }
  struct iotctrl_7seg_disp_connection conn;
  parse_arguments(argc, argv, &gpio_device_path, &data_pin, &chain_count);
  conn.data_pin_num = data_pin;
  conn.clock_pin_num = 11;
  conn.latch_pin_num = 18;
  conn.chain_num = chain_count;
  conn.refresh_rate_hz = 500;
  strcpy(conn.gpiochip_path, gpio_device_path);

  printf("Parameters:\n");
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
  const float values[][2] = {
      {-99.9, -99.9}, {-8.8, -8.8},   {-0.7, -0.7}, {-6.6, -6.6},
      {-55.5, -55.5}, {-4.4, -4.4},   {-0.3, -0.3}, {-2.2, -2.2},
      {-11.1, -11.1}, {0, 0},         {0.1, 0.1},   {2.2, 2.2},
      {33.3, 33.3},   {444.4, 444.4}, {55.5, 55.5}, {6.6, 6.6},
      {0.7, 0.7},     {8.8, 8.8},     {99.9, 99.9}};
  const size_t len = sizeof(values) / sizeof(values[0]);

  while (!ev_flag) {
    for (uint8_t i = 0; i < handle->digit_count && !ev_flag; ++i) {
      iotctrl_7seg_disp_update_digit(
          handle, i,
          iotctrl_7seg_disp_chars_table[IOTCTRL_7SEG_DISP_CHARS_ALL]);
    }
    sleep(1);
    for (size_t i = 0; i < len && !ev_flag; ++i) {
      if (chain_count == 2) {
        printf("Now showing: %.1f, %.1f\n", values[i][0], values[i][1]);
        iotctrl_7seg_disp_update_as_four_digit_float(handle, values[i][0], 0);
        iotctrl_7seg_disp_update_as_four_digit_float(handle, values[i][1], 1);
      } else {
        printf("Now showing: %.1f\n", values[i][0]);
        iotctrl_7seg_disp_update_as_four_digit_float(handle, values[i][0], 0);
      }
      sleep(2);
    }
  }
  printf("Done\n");
  iotctrl_7seg_disp_destory(handle);
err_signal_handler_install:
  return retval;
}
