#ifndef LIBIOTCTRL_BUZZER_H
#define LIBIOTCTRL_BUZZER_H

#include <stdlib.h>

struct buzz_unit {
  int on_off;
  size_t duration_ms;
};

/**
 * @brief Make a buzzer buzz!
 * @param gpiochip_path GPIO device path, typically /dev/gpiochip0
 * @param pin_num Following the numbering of GPIO/BCM schema
 * @param sequence Sequence of beeps
 * @param sequence_len Length of the Sequence array
 * @returns 0 means success or an error code will be returned.
 * */
int iotctrl_make_a_buzz(const char *gpiochip_path, const size_t pin_num,
                        const struct buzz_unit sequence[],
                        const size_t sequence_len);

#endif // LIBIOTCTRL_BUZZER_H
