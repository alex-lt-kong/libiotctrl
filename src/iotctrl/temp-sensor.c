#include "iotctrl/temp-sensor.h"

#include <modbus/modbus.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t calculate_crc(const uint8_t *buf, size_t len) {
  uint16_t crc = 0xFFFF;

  for (size_t pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos]; // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) { // Loop over each bit
      if ((crc & 0x0001) != 0) {   // If the LSB is set
        crc >>= 1;                 // Shift right and XOR 0xA001
        crc ^= 0xA001;
      } else       // Else LSB is not set
        crc >>= 1; // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or
  // swap bytes)
  return crc;
}

int16_t iotctrl_get_temperature(const char *sensor_path,
                                const int enable_debug_output) {
  modbus_t *ctx = NULL;
  int16_t temp = INVALID_TEMP;
  ctx = modbus_new_rtu(sensor_path, 9600, 'N', 8, 1);
  if (ctx == NULL) {
    fprintf(stderr, "Unable to create the libmodbus context\n");
    goto finally;
  }

  modbus_set_slave(ctx, 1);
  modbus_set_debug(ctx, enable_debug_output == 1);
  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    goto finally;
  }
  uint8_t raw_req[] = {0x01, 0x04, 0x04, 0x00, 0x00, 0x01};
  // clang-format off
  // Note that we have to truncate the bytes series from 8 to 6 to make it work.
  // Page 12 of the manufacturer manual documents the format of command bytes format:
  // 1 byte:  device address
  // 1 byte:  function code
  // 2 bytes: register address
  // 2 bytes: register number
  // 2 bytes: CRC
  // clang-format on

  uint8_t rsp[MODBUS_RTU_MAX_ADU_LENGTH];

  int req_length = modbus_send_raw_request(
      ctx, raw_req, sizeof(raw_req) / sizeof(raw_req[0]));
  if (req_length == -1) {
    fprintf(stderr, "Failed to send raw request: %s\n", modbus_strerror(errno));
    goto finally;
  }
  int rc = modbus_receive_confirmation(ctx, rsp);
  if (rc == -1) {
    fprintf(stderr, "Failed to receive a confirmation request: %s\n",
            modbus_strerror(errno));
    goto finally;
  }
  // clang-format off
  // Page 12 of the manufacturer manual documents the format of reply bytes format:
  // 1 byte:  device address
  // 1 byte:  function code
  // 1 byte:  number of bytes (N)
  // N bytes: register number
  // 2 bytes: CRC
  // clang-format on

  if (rsp[0] == 0x01 || rsp[1] == 0x04 || rsp[2] == 0x02) {
    temp = ((rsp[3] << 8) + rsp[4]);
    if (temp == INVALID_TEMP) {
      fprintf(stderr,
              "Reading is INVALID_TEMP(%d). The sensor might be non-existent "
              "or malfunctional\n",
              INVALID_TEMP);
    }
    uint16_t calculated_crc = calculate_crc(rsp, 5);
    uint16_t expected_crc = (rsp[6] << 8) + rsp[5];
    if (calculated_crc != expected_crc) {
      fprintf(stderr, "CRC value does not match!\n");
    }
  } else {
    fprintf(stderr,
            "Invalid response header, expecting 0x01, 0x04, 0x02, but gets "
            "%#04x, %#04x, %#04x\n",
            rsp[0], rsp[1], rsp[2]);
  }

finally:
  if (ctx != NULL) {
    // Can close after checking modbus_connect(ctx) == -1 again:
    // an established connection could not be established one more time, causing
    // the test to fail and left the context open.
    modbus_close(ctx);
    modbus_free(ctx);
  }
  return temp;
}
