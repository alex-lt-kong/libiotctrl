from iotctrl import get_temperature, INVALID_TEMP

import argparse

parser = argparse.ArgumentParser(description='Temp Sensor Tool')
parser.add_argument('-d', '--device-path', required=True,
                    help='path of the device, e.g. /dev/ttyUSB0')
parser.add_argument('-v', '--verbose', action='store_true',
                    help='enable verbose mode')

args = parser.parse_args()
print(get_temperature(args.device_path, args.verbose))
