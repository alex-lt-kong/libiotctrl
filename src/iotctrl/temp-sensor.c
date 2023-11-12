#include "temp-sensor.h"

#include <modbus/modbus.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

const uint16_t iotctrl_invalid_temp = IOTCTRL_INVALID_TEMP;

// This function can also be found at page 21 of
// https://github.com/alex-lt-kong/libiotctrl/blob/main/assets/dl11-mc_manual.pdf
static uint16_t calculate_crc16(const uint8_t *buf, size_t len) {
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

int iotctrl_get_temperature(const char *sensor_path, uint8_t sensor_count,
                            int16_t *readings, const int enable_debug_output) {
  int ret = 0;
  modbus_t *mb_ctx = NULL;
  mb_ctx = modbus_new_rtu(sensor_path, 9600, 'N', 8, 1);
  if (mb_ctx == NULL) {
    fprintf(stderr, "Unable to create the libmodbus context\n");
    ret = -1;
    goto finally;
  }

  modbus_set_slave(mb_ctx, 1);
  modbus_set_debug(mb_ctx, enable_debug_output == 1);
  if (modbus_connect(mb_ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    ret = -2;
    goto finally;
  }
  uint8_t raw_req[] = {0x01, 0x04, 0x04, 0x00, 0x00, sensor_count};
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
      mb_ctx, raw_req, sizeof(raw_req) / sizeof(raw_req[0]));
  if (req_length == -1) {
    fprintf(stderr, "Failed to send raw request: %s\n", modbus_strerror(errno));
    ret = -3;
    goto finally;
  }
  int rc = modbus_receive_confirmation(mb_ctx, rsp);
  if (rc == -1) {
    fprintf(stderr, "Failed to receive a confirmation request: %s\n",
            modbus_strerror(errno));
    ret = -4;
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
    for (uint8_t i = 0; i < sensor_count; ++i) {
      readings[i] = ((rsp[3 + i * 2] << 8) + rsp[4 + i * 2]);
      if (readings[i] == IOTCTRL_INVALID_TEMP) {
        fprintf(stderr,
                "Sensor no. %u (index from 0) returns is INVALID_TEMP(%d). The "
                "sensor might be non-existent or malfunctional\n",
                i + 1, IOTCTRL_INVALID_TEMP);
        ret = -5;
        goto finally;
      }
      uint16_t calculated_crc = calculate_crc16(rsp, 3 + sensor_count * 2);
      uint16_t expected_crc =
          (rsp[4 + sensor_count * 2] << 8) + rsp[3 + sensor_count * 2];
      if (calculated_crc != expected_crc) {
        fprintf(stderr, "CRC value does not match!\n");
        ret = -6;
        goto finally;
      }
    }
  } else {
    ret = -7;
    fprintf(stderr,
            "Invalid response header, expecting 0x01, 0x04, 0x02, but gets "
            "%#04x, %#04x, %#04x\n",
            rsp[0], rsp[1], rsp[2]);
  }

finally:
  if (mb_ctx != NULL) {
    // Can close after checking modbus_connect(ctx) == -1 again:
    // an established connection could not be established one more time, causing
    // the test to fail and left the context open.
    modbus_close(mb_ctx);
    modbus_free(mb_ctx);
  }
  return ret;
}
