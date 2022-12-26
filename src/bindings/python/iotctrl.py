import ctypes as ct
import os


so_file_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'libiotctrl.so')
iotctrl = ct.CDLL(so_file_path)
INVALID_TEMP = ct.c_int.in_dll(iotctrl, "invalid_temp").value

iotctrl.control_relay.argtypes = [ct.c_char_p, ct.c_bool]
iotctrl.control_relay.restype = ct.c_int
iotctrl.get_temperature.argtypes = [ct.c_char_p, ct.c_int]
iotctrl.get_temperature.restype = ct.c_int

def get_temperature(device_path: str, debug_mode: int) -> int:
    temp = iotctrl.get_temperature(ct.create_string_buffer(str.encode(device_path)), debug_mode)
    if temp == INVALID_TEMP:
        raise RuntimeError('Failed to read temperature (some error messages could be sent to stderr)')
    return temp / 10.0


def set_relay(relay_path: str, state: bool) -> None:
    turn_on = ct.c_bool()
    turn_on.value = state
    return iotctrl.control_relay(ct.create_string_buffer(str.encode(relay_path)), turn_on)
