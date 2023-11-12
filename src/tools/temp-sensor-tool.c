#include <iotctrl/temp-sensor.h>

#include <getopt.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_help_then_exit() {

  // clang-format off
  printf("Usage: temp-sensor-tool\n"
         "    -d, --device-path  <device_path>  The path of the device, typically /dev/ttyUSB0\n"
         "    -c, --sensor-count <number>       The number of sensors from DL11-MC series devices, typical numbers are 1 or 2\n"
         "    [-v, --verbose]                   Enable verbose mode\n");
  // clang-format on
  _exit(0);
}

void parse_arguments(int argc, char **argv, char **device_path,
                     uint8_t *sensor_count, bool *verbose_mode) {
  int c;
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
  while (1) {
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"device-path", required_argument, 0, 'd'},
        {"sensor-count", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, NULL, 0}};
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "d:h:c:v", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
    switch (c) {
    case 'd':
      *device_path = optarg;
      break;
    case 'c':
      *sensor_count = atoi(optarg);
      break;
    case 'v':
      *verbose_mode = true;
      break;
    default:
      print_help_then_exit();
    }
  }
  if (*device_path == NULL || *sensor_count == 0) {
    print_help_then_exit();
  }
}

int main(int argc, char **argv) {
  char *device_path = NULL;
  bool verbose_mode = false;
  uint8_t sensor_count = 0;
  parse_arguments(argc, argv, &device_path, &sensor_count, &verbose_mode);
  int16_t readings[sensor_count];
  if (iotctrl_get_temperature(device_path, sensor_count, readings,
                              verbose_mode) != 0) {
    fprintf(stderr, "iotctrl_get_temperature() returns non-zero code\n");
  } else {
    for (uint8_t i = 0; i < sensor_count; ++i) {
      float temp_parsed = readings[i] / 10.0;
      printf("%.1f °C\n", temp_parsed);
    }
  }
  return 0;
}
