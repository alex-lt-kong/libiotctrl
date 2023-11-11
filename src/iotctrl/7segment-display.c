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

// TODO: possible makin iotctrl_ev_flag private?
static sig_atomic_t volatile ev_flag = 0;
static pthread_t th_display_refresh = 0;
static _Atomic uint8_t *per_digit_values = NULL;
static _Atomic uint8_t *per_digit_dots = NULL;

static size_t digit_count = 8;

static int data;
static int clk;
static int latch;
static int chain;

static struct gpiod_chip *chip = NULL;
static struct gpiod_line *line_data = NULL;
static struct gpiod_line *line_clk = NULL;
static struct gpiod_line *line_latch = NULL;

static void push_bit(bool bit) {
  gpiod_line_set_value(line_clk, 0);
  gpiod_line_set_value(line_data, bit);
  gpiod_line_set_value(line_clk, 1);
}

static bool get_bit(uint16_t value, int n) {
  if (value & (1 << n)) {
    return true;
  } else {
    return false;
  }
}
static void write_data_to_register(uint16_t value) {

  for (int i = 8 * chain - 1; i >= 0; --i) {
    push_bit(get_bit(value, i));
  }
}

static uint8_t handle_dot(uint8_t value, bool turn_it_on) {
  return turn_it_on ? value & 0b01111111 : value;
}

void iotctrl_finalize_7seg_display(void) {
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

  free(per_digit_values);
  free(per_digit_dots);
}

// CanNOT expose this to users, it creates intermediate states
static void update_value_one_four_digit_float(float val, uint16_t start_idx) {
  if (val > 1000 || val < -100)
    val = 0;

  per_digit_dots[start_idx + 2] = 1;

  bool still_zero = true;
  if (val >= 0) {
    per_digit_values[start_idx + 0] = (int)fabs(val) % 1000 / 100;
    if (per_digit_values[start_idx + 0] != 0)
      still_zero = false;
    else
      per_digit_values[start_idx + 0] = 10;
  } else {
    per_digit_values[start_idx + 0] = 11;
  }
  per_digit_values[start_idx + 1] = (int)fabs(val) % 100 / 10;
  if (per_digit_values[start_idx + 1] == 0) {
    if (still_zero)
      per_digit_values[start_idx + 1] = 10;
  } else {
    still_zero = false;
  }
  per_digit_values[start_idx + 2] = (int)fabs(val) % 10;
  per_digit_values[start_idx + 3] = (int)fabs(val * 10) % 10;
}

void iotctrl_update_value_two_four_digit_floats(float first, float second) {
  if (first > 1000 || first < -100)
    first = 0;
  if (second > 1000 || second < -100)
    second = 0;

  memset(per_digit_values, 0, digit_count);
  memset(per_digit_dots, 0, digit_count);
  update_value_one_four_digit_float(first, 0);
  update_value_one_four_digit_float(second, 4);
}

static int update_display() {

  for (size_t i = 0; i < digit_count; ++i) {
    write_data_to_register(
        handle_dot(chars_table[per_digit_values[i]], per_digit_dots[i]) << 8 |
        1 << (digit_count - 1 - i));

    gpiod_line_set_value(line_latch, 1);
    gpiod_line_set_value(line_latch, 0);
    usleep(20);
  }

  return 0;
}

static void *ev_display_refresh_thread(void *_) {
  (void)_;
  while (!ev_flag) {
    update_display();
    usleep(10);
  }
  return NULL;
}

int iotctrl_init_display(const char *gpiochip_path,
                         const struct iotctrl_7seg_display_connection conn) {

  digit_count = conn.display_digit_count;

  data = conn.data_pin_num;
  clk = conn.clock_pin_num;
  latch = conn.latch_pin_num;
  chain = conn.chain_num;

  per_digit_values = calloc(sizeof(_Atomic(uint8_t)), digit_count);
  if (per_digit_values == NULL) {
    perror("calloc()");
    iotctrl_finalize_7seg_display();
    return -5;
  }
  per_digit_dots = calloc(sizeof(_Atomic(uint8_t)), digit_count);
  if (per_digit_dots == NULL) {
    perror("calloc()");
    iotctrl_finalize_7seg_display();
    return -5;
  }
  // Per
  // https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/tree/lib/chip.c
  // Internally it uses fopen()/malloc()/ioctl() and all of them set errno on
  // error
  chip = gpiod_chip_open(gpiochip_path);

  if (!chip) {
    fprintf(stderr, "gpiod_chip_open(%s) failed: %d(%s)\n", gpiochip_path,
            errno, strerror(errno));
    return -1;
  }

  line_data = gpiod_chip_get_line(chip, data);
  line_clk = gpiod_chip_get_line(chip, clk);
  line_latch = gpiod_chip_get_line(chip, latch);
  if (line_data == NULL || line_clk == NULL || line_latch == NULL) {
    fprintf(stderr, "gpiod_chip_get_line() failed %d(%s)\n", errno,
            strerror(errno));
    iotctrl_finalize_7seg_display();
    return -2;
  }

  if (gpiod_line_request_output(line_data, "7-segment-display", 0) != 0 ||
      gpiod_line_request_output(line_clk, "7-segment-display", 0) != 0 ||
      gpiod_line_request_output(line_latch, "7-segment-display", 0) != 0) {
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
    fprintf(stderr, "pthread_create() failed: %d(%s)", errno, strerror(errno));
    iotctrl_finalize_7seg_display();
    return -4;
  }
  return 0;
}
