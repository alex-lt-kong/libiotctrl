import serial
import sys

def relay_control(state: int):
  assert state in [0, 1]
  ser = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    xonxoff=serial.XOFF,
    rtscts=False,
    dsrdtr=False
  )
  bytes_ops = [bytes([0xA0, 0x01, 0x00, 0xA1]), bytes([0xA0, 0x01, 0x01, 0xA2])]

  retry = 0
  max_retry = 10;
  while True:
    retry += 1
    try:
      ser.open()
      print('Opened')
      break
    except Exception as ex:
      print(ex)
      ser.close()
    if retry > max_retry:
      break
  ser.isOpen()

  print("Initializing the device ..")

  ser.write(bytes_ops[state])


  print('Done')

def main(args):
  relay_control(int(args[1]))

if __name__ == '__main__':
  if len(sys.argv) <= 1:
    print('operation not specified')
    exit(1)
  if (sys.argv[1] in ['0', '1']) is False:

    print('operation can either be 0 or 1')
    exit(1)
  main(sys.argv)