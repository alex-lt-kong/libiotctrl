#include <iotctrl/temp-sensor.h>

#include <getopt.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_help_then_exit() {
  printf("Usage: temp-sensor-tool\n"
         "    -d, --device-path <device_path>   The path of the device, "
         "typically /dev/ttyUSB0\n"
         "    [-v, --verbose]                    Enable verbose mode\n");
  _exit(0);
}

void parse_arguments(int argc, char **argv, char **device_path,
                     bool *verbose_mode) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"device-path", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "d:hv", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
    switch (c) {
    case 'd':
      *device_path = optarg;
      break;
    case 'v':
      *verbose_mode = true;
      break;
    default:
      print_help_then_exit();
    }
  }
  if (*device_path == NULL) {
    print_help_then_exit();
  }
}

int main(int argc, char **argv) {
  char *device_path = NULL;
  bool verbose_mode = false;
  parse_arguments(argc, argv, &device_path, &verbose_mode);
  int16_t temp_raw = iotctrl_get_temperature(device_path, verbose_mode);
  if (temp_raw == IOTCTRL_INVALID_TEMP) {
    fprintf(stderr, "failed to read from sensor.\n");
  } else {
    float temp_parsed = temp_raw / 10.0;
    printf("%.1f °C\n", temp_parsed);
  }
  return 0;
}
