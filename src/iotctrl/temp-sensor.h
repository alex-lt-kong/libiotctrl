#ifndef LIBIOTCTRL_TEMP_SENSOR_H
#define LIBIOTCTRL_TEMP_SENSOR_H

#include <stdio.h>

#define INVALID_TEMP 0x7FFF

// So that the variable will be in the so library file, which can be read by
// language bindings

/**
 * @param sensor_path path of the temperature sensor, typically something like
 * "/dev/ttyUSB0"
 * @param enable_debug_output pass 1 to print debug info to stdout/stderr
 * @returns the temperature x 10 reading in degree Celsius or INVALID_TEMP if
 * the function failes to query a reading. E.g., return value of 321 means it
 * is 32.1 °C
 */
int get_temperature(const char *sensor_path, const int enable_debug_output);

#endif /* LIBIOTCTRL_TEMP_SENSOR_H */
