#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <unistd.h>

/**
 * @brief iotctrl_dht31_init() is nothing but opening a file descriptor
 * @returns Same as open(), check `man 2 open` for details
 */
int iotctrl_dht31_init(const char *device_path);

/**
 * @brief Read temperature in Celsius and relative humidity from a DHT31 sensor
 * pointed by the given fd
 * @returns 0 on success and non-zero on error
 */
int iotctrl_dht31_read(const int fd, float *temp_celsius,
                       float *relative_humidity);

void iotctrl_dht31_destroy(const int fd);
