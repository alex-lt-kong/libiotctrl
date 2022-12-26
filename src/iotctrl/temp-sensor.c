#include <stdlib.h>

#include "temp-sensor.h"


int get_temperature(const char* sensor_path, const int enable_debug_output) {
  modbus_t *ctx;
  int temp = INVALID_TEMP;
  ctx = modbus_new_rtu(sensor_path, 9600, 'N', 8, 1);
  if (ctx == NULL) {
      fprintf(stderr, "Unable to create the libmodbus context\n");
      return temp;
  }

  modbus_set_slave(ctx, 1);
  modbus_set_debug(ctx, enable_debug_output == 1);
  if (modbus_connect(ctx) == -1) {
      fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
      modbus_free(ctx);
      return temp;
  }
  uint8_t raw_req[] = { 0x01, 0x04, 0x04, 0x00, 0x00, 0x01 };
  // Note that we have to truncate the bytes series from 8 to 6 to make it work.
  // Page 12 of the manufacturer manual documents the format of command bytes format:
  // 1 byte:  device address
  // 1 byte:  function code
  // 2 bytes: register address
  // 2 bytes: register number
  // 2 bytes: CRC
  
  uint8_t rsp[MODBUS_RTU_MAX_ADU_LENGTH];


  int req_length = modbus_send_raw_request(ctx, raw_req, sizeof(raw_req) / sizeof(raw_req[0]));
  if (req_length == -1) {
      fprintf(stderr, "Failed to send raw request: %s\n", modbus_strerror(errno));
      modbus_close(ctx);
      modbus_free(ctx);
      return temp;
  }
  int rc = modbus_receive_confirmation(ctx, rsp);
  if (rc == -1) {
      fprintf(stderr, "Failed to receive a confirmation request: %s\n", modbus_strerror(errno));
      modbus_close(ctx);
      modbus_free(ctx);
      return temp;
  }
  // Page 12 of the manufacturer manual documents the format of reply bytes format:
  // 1 byte:  device address
  // 1 byte:  function code
  // 1 byte:  number of bytes (N)
  // N bytes: register number
  // 2 bytes: CRC
  if (rsp[0] == 0x01 || rsp[1] == 0x04 || rsp[2] == 0x02) {
    temp = ((rsp[3] << 8) + rsp[4]);
  }
  modbus_close(ctx);
  modbus_free(ctx);
  return temp;
}
