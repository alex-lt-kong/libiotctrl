#include "7segment-display.h"

#include <gpiod.h>

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

sig_atomic_t ev_flag = 0;
pthread_t th_display_refresh = 0;
int values[] = {0, 0};
uint8_t per_digit_values[16];
/*
const int data = 17;
const int clk = 11;
const int latch = 18;
const int chain = 2;
*/
size_t digit_count = 8;

int data;
int clk;
int latch;
int chain;

struct gpiod_chip *chip = NULL;
struct gpiod_line *line_data = NULL;
struct gpiod_line *line_clk = NULL;
struct gpiod_line *line_latch = NULL;

void push_bit(bool bit) {
  gpiod_line_set_value(line_clk, 0);
  gpiod_line_set_value(line_data, bit);
  gpiod_line_set_value(line_clk, 1);
}

bool get_bit(uint16_t value, int n) {
  if (value & (1 << n)) {
    return true;
  } else {
    return false;
  }
}
void write_data_to_register(uint16_t value) {

  for (int i = 8 * chain - 1; i >= 0; --i) {
    push_bit(get_bit(value, i));
  }
}

uint8_t handle_dot(uint8_t value, bool turn_it_on) {
  return turn_it_on ? value & 0b01111111 : value;
}

const uint8_t available_chars[] = {
    0b11000000, // 0
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    0b11111000, // 7
    0b10000000, // 8
    0b10010000, // 9
    0b11111111  // empty
};
void iotctrl_finalize_7seg_display() {
  ev_flag = 1;
  if (th_display_refresh != 0)
    (void)pthread_join(th_display_refresh, NULL);

  if (line_data != NULL)
    gpiod_line_release(line_data);
  if (line_clk != NULL)
    gpiod_line_release(line_clk);
  if (line_latch != NULL)
    gpiod_line_release(line_latch);
  if (chip != NULL)
    gpiod_chip_close(chip);
}

int iotctrl_update_value(int val1, int val2) {
  values[0] = val1;
  values[1] = val2;
  return 0;
}

int update_display(uint8_t *values, bool *with_dots) {

  for (size_t i = 0; i < digit_count; ++i) {
    write_data_to_register(handle_dot(available_chars[values[i]], with_dots[i])
                               << 8 |
                           1 << (digit_count - 1 - i));

    gpiod_line_set_value(line_latch, 1);
    gpiod_line_set_value(line_latch, 0);
    usleep(20);
  }

  return 0;
}

void *ev_display_refresh_thread() {
  while (!ev_flag) {
    bool dots[8] = {0, 0, 1, 0, 0, 0, 1, 0};
    while (!ev_flag) {
      // We need to use an intermediary variable to avoid accessing pl members
      // multiple times; otherwise we can still trigger race condition

      per_digit_values[0] = values[0] % 1000 / 100;
      per_digit_values[1] = values[0] % 100 / 10;
      per_digit_values[2] = values[0] % 10;
      per_digit_values[3] = values[0] * 10 % 10;

      per_digit_values[4] = values[1] % 1000 / 100;
      per_digit_values[5] = values[1] % 100 / 10;
      per_digit_values[6] = values[1] % 10;
      per_digit_values[7] = values[1] * 10 % 10;
      update_display(per_digit_values, dots);
    }
  }
}

int iotctrl_init_display(const char *gpiochip_path,
                         const size_t display_digit_count,
                         const int data_pin_num, const int clock_pin_num,
                         const int latch_pin_num, const int chain_num) {

  digit_count = display_digit_count;

  data = data_pin_num;
  clk = clock_pin_num;
  latch = latch_pin_num;
  chain = chain_num;

  chip = gpiod_chip_open(gpiochip_path);

  if (!chip) {
    fprintf(stderr, "gpiod_chip_open(%s) failed\n", gpiochip_path);
    return -1;
  }

  line_data = gpiod_chip_get_line(chip, data);
  line_clk = gpiod_chip_get_line(chip, clk);
  line_latch = gpiod_chip_get_line(chip, latch);
  if (line_data == NULL || line_clk == NULL || line_latch == NULL) {
    fprintf(stderr, "gpiod_chip_get_line() failed\n");
    iotctrl_finalize_7seg_display();
    return -2;
  }

  if (gpiod_line_request_output(line_data, "gpiod-example", 0) != 0 ||
      gpiod_line_request_output(line_clk, "gpiod-example", 0) != 0 ||
      gpiod_line_request_output(line_latch, "gpiod-example", 0) != 0) {
    fprintf(stderr, "gpiod_line_request_output() failed\n");
    iotctrl_finalize_7seg_display();
    return -3;
  }

  if (gpiod_line_set_value(line_clk, 0) != 0 ||
      gpiod_line_set_value(line_latch, 0) != 0) {
    fprintf(stderr, "gpiod_line_set_value() failed\n");
    iotctrl_finalize_7seg_display();
    return -3;
  }
  if (pthread_create(&th_display_refresh, NULL, ev_display_refresh_thread,
                     NULL) != 0) {
    fprintf(stderr, "pthread_create() failed: %d", errno);
    iotctrl_finalize_7seg_display();
    return -4;
  }
  return 0;
}
