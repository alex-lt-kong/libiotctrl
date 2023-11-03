#include "temp-sensor.h"

#include <modbus/modbus.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int16_t get_temperature(const char *sensor_path,
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
