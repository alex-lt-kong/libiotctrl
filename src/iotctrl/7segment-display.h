#ifndef LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
#define LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H

#include <stddef.h>

int iotctrl_init_display(const char *gpiochip_path,
                         const size_t display_digit_count,
                         const int data_pin_num, const int clock_pin_num,
                         const int latch_pin_num, const int chain_num);

int iotctrl_update_value(int val1, int val2);

void iotctrl_finalize_7seg_display();

#endif // LIBIOTCTRL_SEVEN_SEGMENT_DISPLAY_H
