#ifndef LIBIOTCTRL_TEMP_SENSOR_H
#define LIBIOTCTRL_TEMP_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define IOTCTRL_INVALID_TEMP 0x7FFF
// So that the variable will be in the so library file, which can be read by
// language bindings
extern const uint16_t iotctrl_invalid_temp;

/**
 * @param sensor_path path of the temperature sensor, typically something like
 * "/dev/ttyUSB0"
 * @param sensor_count number of sensors, typically 1 or 2
 * @param readings an pre-allocated array with sensor_count elements where
 * readings from sensors will be saved to. Readings are temperature x 10 in
 * degree Celsius or IOTCTRL_INVALID_TEMP if the function failes to query a
 * sensor. E.g., return value of 321 means it is 32.1 °C
 * @param enable_debug_output pass 1 to print debug info to stdout/stderr
 * @returns 0 on success or an error code. If function returns an error code,
 * readings will be in an unspecified state
 */
int iotctrl_get_temperature(const char *sensor_path, uint8_t sensor_count,
                            int16_t *readings, const int enable_debug_output);

#ifdef __cplusplus
}
#endif

#endif // LIBIOTCTRL_TEMP_SENSOR_H
