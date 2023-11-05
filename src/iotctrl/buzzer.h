#ifndef LIBIOTCTRL_BUZZER_H
#define LIBIOTCTRL_BUZZER_H

#include <stdlib.h>

struct iotctrl_buzz_unit {
  int on_off;
  size_t duration_ms;
};

/**
 * @brief Make a buzzer buzz!
 * @param gpiochip_path GPIO device path, typically /dev/gpiochip0
 * @param signal_pin Signal pin (a.k.a., I/O), following the numbering of
 * GPIO/BCM schema
 * @param sequence Sequence of beeps. Users should always terminate the sequence
 * with an off section (i.e., setting on_off to 0); otherwise the buzzer will
 * keep buzzing
 * @param sequence_len Length of the Sequence array
 * @returns 0 means success or an error code will be returned.
 * */
int iotctrl_make_a_buzz(const char *gpiochip_path, const size_t signal_pin,
                        const struct iotctrl_buzz_unit sequence[],
                        const size_t sequence_len);

#endif // LIBIOTCTRL_BUZZER_H
