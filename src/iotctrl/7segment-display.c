#include "7segment-display.h"

#include <gpiod.h>

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BIT_PER_DIGIT 8
#define DIGIT_PER_MODULE 4

void push_bit(struct iotctrl_7seg_disp_handle handle, bool bit) {
  gpiod_line_set_value(handle.line_clk, 0);
  gpiod_line_set_value(handle.line_data, bit);
  gpiod_line_set_value(handle.line_clk, 1);
}

bool extract_bit(uint16_t value, int pos) { return (value & (1 << pos)); }

void write_single_digit_data_to_register(struct iotctrl_7seg_disp_handle handle,
                                         uint16_t value) {
  for (int i = sizeof(uint16_t) * CHAR_BIT - 1; i >= 0; --i) {
    push_bit(handle, extract_bit(value, i));
  }
}

uint8_t handle_dot(uint8_t value, bool turn_it_on) {
  // Counter-intuitive definition:
  // 0 turns a segment ON, 1 turns a segment OFF
  // The highest digit controls the dot
  return turn_it_on ? value & 0b01111111 : value;
}

void iotctrl_7seg_disp_destroy(struct iotctrl_7seg_disp_handle *handle) {
  if (handle == NULL) return;
  // TODO: There is still a rare race condition, we probably need a mutex to
  // handle it correctly.
  if (handle->ev_flag == 0) {
    handle->ev_flag = 1;
    if (handle->th_display_refresh != 0)
      (void)pthread_join(handle->th_display_refresh, NULL);
  }

  if (handle->line_data != NULL)
    gpiod_line_release(handle->line_data);
  if (handle->line_clk != NULL)
    gpiod_line_release(handle->line_clk);
  if (handle->line_latch != NULL)
    gpiod_line_release(handle->line_latch);
  if (handle->chip != NULL)
    gpiod_chip_close(handle->chip);

  free(handle->digit_values);
  free(handle->per_digit_dots);

  free(handle);
}

void iotctrl_7seg_disp_update_digit(struct iotctrl_7seg_disp_handle *h, int idx,
                                    uint8_t val) {
  h->digit_values[idx] = val;
}

void iotctrl_7seg_disp_update_as_four_digit_float(
    struct iotctrl_7seg_disp_handle *h, float val, int float_idx) {
  uint8_t *table = (uint8_t *)iotctrl_7seg_disp_chars_table;
  if (val > 1000 || val < -100) {
    fprintf(stderr, "float (%f) out of range, reset to 0\n", val);
    val = 0;
  }
  int idx = float_idx * DIGIT_PER_MODULE;
  h->per_digit_dots[idx + 2] = 1;

  if (val >= 0) {

    h->digit_values[idx + 0] = table[(int)fabs(val) % 1000 / 100];
    if (val < 100.00)
      h->digit_values[idx + 0] = table[IOTCTRL_7SEG_DISP_CHARS_EMPTY];
  } else {
    h->digit_values[idx + 0] = table[IOTCTRL_7SEG_DISP_CHARS_MINUS];
  }

  h->digit_values[idx + 1] = table[(int)fabs(val) % 100 / 10];

  if (val < 10 && val > -10)
    h->digit_values[idx + 1] = table[IOTCTRL_7SEG_DISP_CHARS_EMPTY];

  // "& table[IOTCTRL_7SEG_DISP_CHARS_DOT]" means append a dot to the digit
  h->digit_values[idx + 2] =
      table[(int)fabs(val) % 10] & table[IOTCTRL_7SEG_DISP_CHARS_DOT];

  h->digit_values[idx + 3] = table[(int)fabs(val * 10) % 10];
}

int update_display(struct iotctrl_7seg_disp_handle *h) {
  for (int i = 0; i < h->digit_count; ++i) {
    // Position of a digit. E.g., 0b0000010 means the tens place of the
    // 4/8-digit number
    uint16_t position = 1 << (h->digit_count - 1 - i);

    // High 8 bits represent the number; low 8 bits represent the position of
    // the number
    uint16_t val_and_pos = h->digit_values[i] << 8 | position;
    write_single_digit_data_to_register(*h, val_and_pos);
    gpiod_line_set_value(h->line_latch, 1);
    gpiod_line_set_value(h->line_latch, 0);
    usleep(h->refresh_delay_us);
  }

  return 0;
}

void *ev_display_refresh_thread(void *ctx) {
  struct iotctrl_7seg_disp_handle *h = (struct iotctrl_7seg_disp_handle *)ctx;
  // TODO: There is still a rare race condition, we probably need a mutex to
  // handle it correctly.
  h->ev_flag = 0;
  while (!h->ev_flag) {
    update_display(h);
  }
  return NULL;
}

void iotctrl_7seg_disp_turn_on_all_segments(
    struct iotctrl_7seg_disp_handle *handle, int duration_sec) {
  for (uint8_t i = 0; i < handle->digit_count; ++i) {
    iotctrl_7seg_disp_update_digit(
        handle, i, iotctrl_7seg_disp_chars_table[IOTCTRL_7SEG_DISP_CHARS_ALL]);
  }
  sleep(duration_sec);
}

struct iotctrl_7seg_disp_handle *
iotctrl_7seg_disp_init(const struct iotctrl_7seg_disp_connection conn) {

  struct iotctrl_7seg_disp_handle *h =
      malloc(sizeof(struct iotctrl_7seg_disp_handle));
  if (h == NULL) {
    perror("malloc()");
    return NULL;
  }
  if (conn.chain_num != 1 && conn.chain_num != 2) {
    fprintf(stderr, "Invalid chain_num (%d), must be 1 or 2\n", conn.chain_num);
    free(h);
    return NULL;
  }

  h->ev_flag = 1;
  h->data = conn.data_pin_num;
  h->clk = conn.clock_pin_num;
  h->latch = conn.latch_pin_num;
  h->chain = conn.chain_num;
  h->digit_count = h->chain * DIGIT_PER_MODULE;

  h->digit_values = calloc(sizeof(uint8_t), h->digit_count);

  h->refresh_delay_us = 1000 * 1000 / conn.refresh_rate_hz;

  if (h->digit_values == NULL) {
    perror("calloc()");
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }
  h->per_digit_dots = calloc(sizeof(uint8_t), h->digit_count);
  if (h->per_digit_dots == NULL) {
    perror("calloc()");
    iotctrl_7seg_disp_destroy(h);
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

  // We'd better separate these three gpiod_chip_get_line() calls so that in
  // case of incorrect wiring, we will know which wire is incorrectly connected.

  // See if we can refactor this snippet
  h->line_data = NULL;
  h->line_clk = NULL;
  h->line_latch = NULL;
  h->line_data = gpiod_chip_get_line(h->chip, h->data);
  if (h->line_data == NULL) {
    fprintf(stderr, "gpiod_chip_get_line(h->chip, h->data) failed: %d(%s)\n",
            errno, strerror(errno));
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }
  if (gpiod_line_request_output(h->line_data, "7-segment-display", 0) != 0) {
    fprintf(stderr, "gpiod_line_request_output(h->line_data) failed\n");
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }

  h->line_clk = gpiod_chip_get_line(h->chip, h->clk);
  if (h->line_clk == NULL) {
    fprintf(stderr, "gpiod_chip_get_line(h->chip, h->clk) failed: %d(%s)\n",
            errno, strerror(errno));
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }
  if (gpiod_line_request_output(h->line_clk, "7-segment-display", 0) != 0) {
    fprintf(stderr, "gpiod_line_request_output(h->line_clk) failed\n");
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }

  h->line_latch = gpiod_chip_get_line(h->chip, h->latch);
  if (h->line_latch == NULL) {
    fprintf(stderr, "gpiod_chip_get_line(h->chip, h->latch) failed: %d(%s)\n",
            errno, strerror(errno));
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }
  if (gpiod_line_request_output(h->line_latch, "7-segment-display", 0) != 0) {
    fprintf(stderr, "gpiod_line_request_output(h->line_latch) failed\n");
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }
  // See if we can refactor this snippet

  if (gpiod_line_set_value(h->line_clk, 0) != 0 ||
      gpiod_line_set_value(h->line_latch, 0) != 0) {
    fprintf(stderr, "gpiod_line_set_value() failed\n");
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }
  if (pthread_create(&h->th_display_refresh, NULL, ev_display_refresh_thread,
                     h) != 0) {
    fprintf(stderr, "pthread_create() failed: %d(%s)", errno, strerror(errno));
    iotctrl_7seg_disp_destroy(h);
    return NULL;
  }
  return h;
}
