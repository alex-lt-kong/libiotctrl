#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int control_relay(char* relay_path, bool turn_on) {
  FILE *fptr;

  if ((fptr = fopen(relay_path, "r"))) {
    fclose(fptr);
  } else {
    fprintf(stderr, "Failed to open the relay device at %s\n.", relay_path);
    return 1;
  }

  fptr = fopen(relay_path, "wb");
  if(fptr == NULL) {
    fprintf(stderr, "Failed to open the relay device at %s\n.", relay_path);
    return 2;            
  }
  uint8_t off_command[] = {0xA0, 0x01, 0x00, 0xA1};
  uint8_t on_command[] = {0xA0, 0x01, 0x01, 0xA2};
  size_t result = fwrite(turn_on ? on_command : off_command, sizeof(uint8_t), 4, fptr);
  if (result != 4) {
    fprintf(stderr, "Failed to send command to relay, %d bytes, instead of 4 bytes, are written.\n", result);
    return 3;            
  }
  
  fclose(fptr);
  return 0;
}

int main() {
  control_relay("/dev/ttyUSB4", true);
  control_relay("/dev/ttyUSB0", false);
  control_relay("/dev/ttyUSB0", true);
  return 0;
}