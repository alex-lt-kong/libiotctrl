#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <unistd.h>

int iotctrl_dht31_init(const char *device_path) {
  int fd;
  if ((fd = open(device_path, O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open(%s): %d(%s)", device_path, errno,
            strerror(errno));
  }

  // Get I2C device, SHT31 I2C address is 0x44(68)
  if (ioctl(fd, I2C_SLAVE, 0x44) != 0) {
    fprintf(stderr, "Failed to ioctl(%s): %d(%s)", device_path, errno,
            strerror(errno));
  }
  return fd;
}

/**
 * Performs a CRC8 calculation on the supplied values.
 *
 * @param data  Pointer to the data to use when calculating the CRC8.
 * @param len   The number of bytes in 'data'.
 *
 * @return The computed CRC8 value.
 */
uint8_t crc8(const uint8_t *data, int len) {
  // Ref:
  // https://github.com/adafruit/Adafruit_SHT31/blob/bd465b980b838892964d2744d06ffc7e47b6fbef/Adafruit_SHT31.cpp#L163C4-L194

  const uint8_t POLYNOMIAL = 0x31;
  uint8_t crc = 0xFF;

  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}

int iotctrl_dht31_read(const int fd, float *temp_celsius,
                       float *relative_humidity) {

  // Send high repeatability measurement command
  // Command msb, command lsb(0x2C, 0x06)
  uint8_t config[2] = {0x2C, 0x06};
  if (write(fd, config, 2) != 2) {
    fprintf(stderr, "Failed to write() command to fd %d: %d(%s)", fd, errno,
            strerror(errno));
    return -1;
  }

  // Read 6 bytes of data
  // temp msb, temp lsb, temp CRC, humidity msb, humidity lsb,
  // humidity CRC
  uint8_t buf[6] = {0};
  if (read(fd, buf, 6) != 6) {
    fprintf(stderr, "Failed to read() values from fd %d: %d(%s).", fd, errno,
            strerror(errno));
    return -1;
  }
  // Reference:
  // https://github.com/adafruit/Adafruit_SHT31/blob/bd465b980b838892964d2744d06ffc7e47b6fbef/Adafruit_SHT31.cpp#L197C8-L227
  float temp_celsius_t = (((buf[0] << 8) | buf[1]) * 175.0) / 65535.0 - 45.0;
  float relative_humidity_t = ((625 * ((buf[3] << 8) | buf[4])) >> 12) / 100.0;
  if (buf[2] != crc8(buf, 2) || buf[5] != crc8(buf + 3, 2)) {
    fprintf(stderr,
            "Data read from fd %d but CRC8 failed. Retrieved (erroneous) "
            "readings are %f (temperature, °C), %f (relative humidity, %%)",
            fd, temp_celsius_t, relative_humidity_t);
    return -1;
  }

  *temp_celsius = temp_celsius_t;
  *relative_humidity = relative_humidity_t;

  return 0;
}

void iotctrl_dht31_destroy(const int fd) {
  if (fd >= 0)
    close(fd);
  else
    fprintf(stderr, "Warning: trying to destroy an invalid dht31 handle (%d)\n",
            fd);
}
