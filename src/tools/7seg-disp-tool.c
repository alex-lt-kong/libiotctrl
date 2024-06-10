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
         "    -p, --device-path  <device_path> The path of the GPIO device, typically something like /dev/gpiochip0\n"
         "    -d, --data-pin     <pin_number>  The GPIO pin number in GPIO/BCM schema that connects to the DIO pin (default: 17)\n"
         "    -c, --chain-count  <count>       Number of four-digit displays that are daisy chained together, it should typically be 1 or 2\n"
         "    -s, --clock-pin    <pin_number>  The GPIO pin number in GPIO/BCM schema that connects to the SCLK (clock signal) pin (default: 11)\n"
         "    -l, --latch-pin    <pin_number>  The GPIO pin number in GPIO/BCM schema that connects to the RCLK (register clock) (default: 18)\n"
         "    -r, --refresh-rate <rate>        How frequent are single digits being refreshed. (default: 1KHz)\n",
         argv[0]);
  // clang-format on
  _exit(0);
}

void parse_arguments(int argc, char **argv,
                     struct iotctrl_7seg_disp_connection *conn) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"device-path", required_argument, 0, 'p'},
        {"data-pin", required_argument, 0, 'd'},
        {"chain-count", required_argument, 0, 'c'},
        {"clock-pin", required_argument, 0, 's'},
        {"latch-pin", required_argument, 0, 'l'},
        {"refresh-rate", required_argument, 0, 'r'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "p:d:c:s:l:r:h", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
    switch (c) {
    case 'p':
      strncpy(conn->gpiochip_path, optarg, PATH_MAX);
      break;
    case 'd':
      conn->data_pin_num = atoi(optarg);
      break;
    case 'c':
      conn->chain_num = atoi(optarg);
      break;
    case 'o':
      conn->clock_pin_num = atoi(optarg);
      break;
    case 'l':
      conn->latch_pin_num = atoi(optarg);
      break;
    case 'r':
      conn->refresh_rate_hz = atoi(optarg);
      break;
    case 'h':
      print_help_then_exit(argv);
      break;
    default:
      break;
    }
  }
}

int main(int argc, char **argv) {
  int retval = 0;
  if (install_signal_handler() != 0) {
    retval = -1;
    goto err_signal_handler_install;
  }
  struct iotctrl_7seg_disp_connection conn;
  conn.data_pin_num = 17;
  conn.clock_pin_num = 11;
  conn.latch_pin_num = 18;
  conn.chain_num = 2;
  conn.refresh_rate_hz = 1000;
  strcpy(conn.gpiochip_path, "/dev/gpiochip0");
  parse_arguments(argc, argv, &conn);

  printf("Parameters:\n");
  printf("data_pin_num: %d\n", conn.data_pin_num);
  printf("clock_pin_num: %d\n", conn.clock_pin_num);
  printf("latch_pin_num: %d\n", conn.latch_pin_num);
  printf("chain_num: %d\n", conn.chain_num);
  printf("refresh_rate_hz: %d\n", conn.refresh_rate_hz);
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
    int sec = 5;
    printf("Turning on all segments for %d seconds\n", sec);
    iotctrl_7seg_disp_turn_on_all_segments(handle, sec);
    for (size_t i = 0; i < len && !ev_flag; ++i) {
      if (conn.chain_num == 2) {
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
