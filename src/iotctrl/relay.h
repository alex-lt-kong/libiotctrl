#ifndef LIBIOTCTRL_RELAY_H
#define LIBIOTCTRL_RELAY_H

#include <stdbool.h>

/**
 * @brief switch on/off a relay by path
 * @param relay_path the path of the relay
 * @param turn_on turn the relay on or off
 * @returns 0 means success or an error code will be returned.
 * */

int control_relay(const char *relay_path, bool turn_on);

#endif /* LIBIOTCTRL_RELAY_H */
