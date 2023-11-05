#ifndef LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
#define LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H

#include <stddef.h>
#include <stdint.h>

extern const uint8_t iotctrl_chars_table[];

int iotctrl_init_display(const char *gpiochip_path,
                         const size_t display_digit_count,
                         const int data_pin_num, const int clock_pin_num,
                         const int latch_pin_num, const int chain_num);

/**
 * @brief A convenient wrapper to print two floats with one decimal place to the
 * 7-seg displsy. The valid range is [-99.9, 999.9]. Invalid input will be reset
 * to 0.0
 * @param first first float
 * @param second second float
 * */
void iotctrl_update_value_two_four_digit_floats(float first, float second);

void iotctrl_finalize_7seg_display();

#endif // LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
