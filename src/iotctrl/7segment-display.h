#ifndef LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
#define LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H

#include <stddef.h>
#include <stdint.h>

struct iotctrl_7seg_display_connection {
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
};

/**
 * @brief Initialize a 7seg display
 * @param gpiochip_path GPIO chip's path, typically something like
 * /dev/gpiochip0
 * @param conn a struct that defines the pin connections/etc of a 7seg display
 * @returns 0 on success or an error code. libgpio does not guarantee that its
 * functions set errno code, but many of its functions do set it.
 * */
int iotctrl_init_display(const char *gpiochip_path,
                         const struct iotctrl_7seg_display_connection conn);

/**
 * @brief A convenient wrapper to print two floats with one decimal place to the
 * 7-seg displsy. The valid range is [-99.9, 999.9]. Invalid input will be reset
 * to 0.0
 * @param first first float
 * @param second second float
 * */
void iotctrl_update_value_two_four_digit_floats(float first, float second);

/**
 * @brief Release GPIO resources and internal thread after the 7seg display is
 * not needed
 * */
void iotctrl_finalize_7seg_display();

#endif // LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
