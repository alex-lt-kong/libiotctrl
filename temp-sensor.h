#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdio.h>
#include <modbus/modbus.h>
#include <errno.h>

#define INVALID_TEMP 0x7FFF

/**
 * @param sensor_path path of the temperature sensor, typically something like "/dev/ttyUSB0"
 * @param enable_debug_output pass 1 to print debug info to stdout/stderr
 * @returns the temperature x 10 reading in degree Celsius or INVALID_TEMP if the function failes to query a reading.
 * E.g., return value of 321 means it is 32.1 °C
*/
int get_temperature(const char* sensor_path, const int enable_debug_output);

#endif /* TEMP_SENSOR_H */