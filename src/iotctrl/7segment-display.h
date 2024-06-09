#ifndef LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
#define LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H

#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Counter-intuitive definition:
// 0 turns a segment ON, 1 turns a segment OFF
// The highest digit controls the dot
static const uint8_t iotctrl_7seg_disp_chars_table[] = {
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
    0b10111111, // minus
    0b01111111, // dot
    0b00000000  // all
};

#define IOTCTRL_7SEG_DISP_CHARS_EMPTY 10
#define IOTCTRL_7SEG_DISP_CHARS_MINUS 11
#define IOTCTRL_7SEG_DISP_CHARS_DOT 12
#define IOTCTRL_7SEG_DISP_CHARS_ALL 13

// Connection details needed to control a 7-segent display device
struct iotctrl_7seg_disp_connection {
  // a.k.a. DIO (data input/output)
  uint8_t data_pin_num;
  // a.k.a. SCLK (clock signal)
  uint8_t clock_pin_num;
  // a.k.a. RCLK (register clock)
  uint8_t latch_pin_num;
  // represents the number of display modules connected in a daisy-chain
  // configuration. Currently we use 2 4-digit displays, so this value can only
  // be 2
  uint8_t chain_num;
  char gpiochip_path[PATH_MAX + 1];
  // How frequent is single digit being refreshed.
  // This parameter is highly hardware-dependent and one may need to take a
  // trial-and-error approach to ascertain the balance between minimal flashing
  // display and CPU use. A good starting point is 1KHz then plus/minus by a
  // factor of 2
  uint16_t refresh_rate_hz;
};

struct iotctrl_7seg_disp_handle {
  sig_atomic_t volatile ev_flag;
  pthread_t th_display_refresh;
  uint8_t *digit_values;
  uint8_t *per_digit_dots;

  int data;
  int clk;
  int latch;
  int chain;
  int digit_count;

  struct gpiod_chip *chip;
  struct gpiod_line *line_data;
  struct gpiod_line *line_clk;
  struct gpiod_line *line_latch;

  uint32_t refresh_delay_us;
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
 * @brief Update the zero-based `idx` digit of the 7-segment digital tube
 * @param h Handle
 * @param idx The zero-based index, representing which digit you want to update
 * @param val A bit array representing which segment(s) of a digit you want
 to turn on/off. You may refer to the `iotctrl_7seg_disp_chars_table` for some
 hints
 * */
void iotctrl_7seg_disp_update_digit(struct iotctrl_7seg_disp_handle *h, int idx,
                                    uint8_t val);
/**
 * @brief The 7-segment display's digits are grouped as four-digit
 * fixed-point float with one decimal place. This function is a convenient
 * way to update the display with such float input.
 * @param val The float object to be shown. Its valid range is [-99.9,
 * 999.9]. Invalid input will be reset to 0.0
 * @param float_idx The zero-index pointer denoting which float is to be
 * updated. For a 8-digit digital tube, float_idx can be either 0 or 1.
 * */
void iotctrl_7seg_disp_update_as_four_digit_float(
    struct iotctrl_7seg_disp_handle *h, float val, int float_idx);

/**
 * @brief Release GPIO resources and internal thread after the 7seg display is
 * not needed
 * */
void iotctrl_7seg_disp_destory(struct iotctrl_7seg_disp_handle *handle);

#endif // LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
