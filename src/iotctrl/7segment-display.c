#include "7segment-display.h"

#include <gpiod.h>

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// 0 turns a segment on, 1 turns a segment off
// The highest digit controls the dot
static const uint8_t chars_table[] = {
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
    0b11111111, // empty
    0b10111111  // -
};

void push_bit(struct iotctrl_7seg_disp_handle handle, bool bit) {
  gpiod_line_set_value(handle.line_clk, 0);
  gpiod_line_set_value(handle.line_data, bit);
  gpiod_line_set_value(handle.line_clk, 1);
}

bool get_bit(uint16_t value, int n) {
  if (value & (1 << n)) {
    return true;
  } else {
    return false;
  }
}

void write_data_to_register(struct iotctrl_7seg_disp_handle handle,
                            uint16_t value) {
  for (int i = 8 * handle.chain - 1; i >= 0; --i) {
    push_bit(handle, get_bit(value, i));
  }
}

uint8_t handle_dot(uint8_t value, bool turn_it_on) {
  return turn_it_on ? value & 0b01111111 : value;
}

void iotctrl_7seg_disp_destory(struct iotctrl_7seg_disp_handle *handle) {
  handle->ev_flag = 1;
  if (handle->th_display_refresh != 0)
    (void)pthread_join(handle->th_display_refresh, NULL);

  if (handle->line_data != NULL)
    gpiod_line_release(handle->line_data);
  if (handle->line_clk != NULL)
    gpiod_line_release(handle->line_clk);
  if (handle->line_latch != NULL)
    gpiod_line_release(handle->line_latch);
  if (handle->chip != NULL)
    gpiod_chip_close(handle->chip);

  free(handle->per_digit_values);
  free(handle->per_digit_dots);
}

// CanNOT expose this to users, it creates intermediate states
/**
 * @brief The 8-digit digital tube is considered two four-digit fixed point
 * float with one decimal place
 * @param handle The handle used to manipulate the corresponding display
 * @param val Value to be displayed
 * @param start_idx The index of the first digit of one four-digit float
 */
void update_value_one_four_digit_float(struct iotctrl_7seg_disp_handle handle,
                                       float val, uint16_t start_idx) {
  if (val > 1000 || val < -100)
    val = 0;

  handle.per_digit_dots[start_idx + 2] = 1;

  bool still_zero = true;
  if (val >= 0) {
    handle.per_digit_values[start_idx + 0] = (int)fabs(val) % 1000 / 100;
    if (handle.per_digit_values[start_idx + 0] != 0)
      still_zero = false;
    else
      handle.per_digit_values[start_idx + 0] = 10;
  } else {
    handle.per_digit_values[start_idx + 0] = 11;
  }
  handle.per_digit_values[start_idx + 1] = (int)fabs(val) % 100 / 10;
  if (handle.per_digit_values[start_idx + 1] == 0) {
    if (still_zero)
      handle.per_digit_values[start_idx + 1] = 10;
  } else {
    still_zero = false;
  }
  handle.per_digit_values[start_idx + 2] = (int)fabs(val) % 10;
  handle.per_digit_values[start_idx + 3] = (int)fabs(val * 10) % 10;
}

void iotctrl_7seg_disp_update_as_four_digit_float(
    struct iotctrl_7seg_disp_handle h, float val, int float_idx) {
  if (val > 1000 || val < -100) {
    fprintf(stderr, "float (%f) out of range, reset to 0\n", val);
    val = 0;
  }
  int idx = float_idx * 4;
  h.per_digit_dots[idx + 2] = 1;

  bool still_zero = true;
  if (val >= 0) {
    h.per_digit_values[idx + 0] = (int)fabs(val) % 1000 / 100;
    if (h.per_digit_values[idx + 0] != 0)
      still_zero = false;
    else
      h.per_digit_values[idx + 0] = 10;
  } else {
    h.per_digit_values[idx + 0] = 11;
  }
  h.per_digit_values[idx + 1] = (int)fabs(val) % 100 / 10;
  if (h.per_digit_values[idx + 1] == 0) {
    if (still_zero)
      h.per_digit_values[idx + 1] = 10;
  } else {
    still_zero = false;
  }
  h.per_digit_values[idx + 2] = (int)fabs(val) % 10;
  h.per_digit_values[idx + 3] = (int)fabs(val * 10) % 10;
  /*
    memset(h.per_digit_values, 0, h.digit_count);
    memset(h.per_digit_dots, 0, h.digit_count);
    update_value_one_four_digit_float(h, first, 0);
    update_value_one_four_digit_float(h, second, 4);*/
}

int update_display(struct iotctrl_7seg_disp_handle *h) {

  for (size_t i = 0; i < h->digit_count; ++i) {
    write_data_to_register(
        *h,
        handle_dot(chars_table[h->per_digit_values[i]], h->per_digit_dots[i])
                << 8 |
            1 << (h->digit_count - 1 - i));

    gpiod_line_set_value(h->line_latch, 1);
    gpiod_line_set_value(h->line_latch, 0);
    usleep(h->refresh_delay_us);
  }

  return 0;
}

void *ev_display_refresh_thread(void *ctx) {
  struct iotctrl_7seg_disp_handle *h = (struct iotctrl_7seg_disp_handle *)ctx;
  while (!h->ev_flag) {
    update_display(h);
  }
  return NULL;
}

struct iotctrl_7seg_disp_handle *
iotctrl_7seg_disp_init(const struct iotctrl_7seg_disp_connection conn) {

  struct iotctrl_7seg_disp_handle *h =
      malloc(sizeof(struct iotctrl_7seg_disp_handle));
  if (h == NULL) {
    perror("malloc()");
    return NULL;
  }
  h->digit_count = conn.display_digit_count;

  h->data = conn.data_pin_num;
  h->clk = conn.clock_pin_num;
  h->latch = conn.latch_pin_num;
  h->chain = conn.chain_num;

  h->per_digit_values = calloc(sizeof(_Atomic(uint8_t)), h->digit_count);

  h->refresh_delay_us = 1000 * 1000 / conn.refresh_rate_hz;

  if (h->per_digit_values == NULL) {
    perror("calloc()");
    iotctrl_7seg_disp_destory(h);
    return NULL;
  }
  h->per_digit_dots = calloc(sizeof(_Atomic(uint8_t)), h->digit_count);
  if (h->per_digit_dots == NULL) {
    perror("calloc()");
    iotctrl_7seg_disp_destory(h);
    return NULL;
  }
  // Per
  // https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/tree/lib/chip.c
  // Internally it uses fopen()/malloc()/ioctl() and all of them set errno on
  // error
  h->chip = gpiod_chip_open(conn.gpiochip_path);

  if (!h->chip) {
    fprintf(stderr, "gpiod_chip_open(%s) failed: %d(%s)\n", conn.gpiochip_path,
            errno, strerror(errno));
    return NULL;
  }

  h->line_data = gpiod_chip_get_line(h->chip, h->data);
  h->line_clk = gpiod_chip_get_line(h->chip, h->clk);
  h->line_latch = gpiod_chip_get_line(h->chip, h->latch);
  if (h->line_data == NULL || h->line_clk == NULL || h->line_latch == NULL) {
    fprintf(stderr, "gpiod_chip_get_line() failed %d(%s)\n", errno,
            strerror(errno));
    iotctrl_7seg_disp_destory(h);
    return NULL;
  }

  if (gpiod_line_request_output(h->line_data, "7-segment-display", 0) != 0 ||
      gpiod_line_request_output(h->line_clk, "7-segment-display", 0) != 0 ||
      gpiod_line_request_output(h->line_latch, "7-segment-display", 0) != 0) {
    fprintf(stderr, "gpiod_line_request_output() failed\n");
    iotctrl_7seg_disp_destory(h);
    return NULL;
  }

  if (gpiod_line_set_value(h->line_clk, 0) != 0 ||
      gpiod_line_set_value(h->line_latch, 0) != 0) {
    fprintf(stderr, "gpiod_line_set_value() failed\n");
    iotctrl_7seg_disp_destory(h);
    return NULL;
  }
  if (pthread_create(&h->th_display_refresh, NULL, ev_display_refresh_thread,
                     h) != 0) {
    fprintf(stderr, "pthread_create() failed: %d(%s)", errno, strerror(errno));
    iotctrl_7seg_disp_destory(h);
    return NULL;
  }
  return h;
}
