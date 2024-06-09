#ifndef LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
#define LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H

#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>

// Connection details needed to control a 7-segent display device
struct iotctrl_7seg_disp_connection {
  // Number of digits of the 7-segment display, currently only 8 is supported
  size_t display_digit_count;
  // a.k.a. DIO (data input/output)
  int data_pin_num;
  // a.k.a. SCLK (clock signal)
  int clock_pin_num;
  // a.k.a. RCLK (register clock)
  int latch_pin_num;
  // represents the number of display modules connected in a daisy-chain
  // configuration. Currently we use 2 4-digit displays, so this value can only
  // be 2
  int chain_num;
  char gpiochip_path[PATH_MAX + 1];
};

struct iotctrl_7seg_disp_handle {
  sig_atomic_t volatile ev_flag;
  pthread_t th_display_refresh;
  _Atomic uint8_t *per_digit_values;
  _Atomic uint8_t *per_digit_dots;

  size_t digit_count;

  int data;
  int clk;
  int latch;
  int chain;

  struct gpiod_chip *chip;
  struct gpiod_line *line_data;
  struct gpiod_line *line_clk;
  struct gpiod_line *line_latch;
};

/**
 * @brief Initialize a 7seg display
 * @param gpiochip_path GPIO chip's path, typically something like
 * /dev/gpiochip0. iotctrl_7seg_disp_init() does not take ownership of this
 * variable.
 * @param conn a struct that defines the pin connections/etc of a 7seg display
 * @returns 0 on success or an error code. libgpio does not guarantee that its
 * functions set errno code, but many of its functions do set it.
 * */
struct iotctrl_7seg_disp_handle *
iotctrl_7seg_disp_init(const struct iotctrl_7seg_disp_connection conn);

/**
 * @brief A convenient wrapper to print two floats with one decimal place to the
 * 7-seg displsy. The valid range is [-99.9, 999.9]. Invalid input will be reset
 * to 0.0
 * @param first first float
 * @param second second float
 * */
void iotctrl_7seg_disp_update_2floats(struct iotctrl_7seg_disp_handle handle,
                                      float first, float second);

/**
 * @brief Release GPIO resources and internal thread after the 7seg display is
 * not needed
 * */
void iotctrl_7seg_disp_destory(struct iotctrl_7seg_disp_handle *handle);

#endif // LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
