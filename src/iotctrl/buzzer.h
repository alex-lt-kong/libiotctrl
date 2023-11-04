#include <stdlib.h>

struct buzz_unit {
  int on_off;
  size_t duration_ms;
};

/**
 * @brief Make a beeper beep
 * @param gpiochip_num Number of the gpiochip, pass 0 if you are not sure
 * @param pin_num Following the numbering of GPIO/BCM schema
 * @param sequence Sequence of beeps
 * @param sequence_len Length of the Sequence array
 * @returns 0 means success or an error code will be returned.
 * */
int make_a_buzz(const unsigned int gpiochip_num, const size_t pin_num,
                const struct buzz_unit sequence[], const size_t sequence_len);
