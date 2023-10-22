# libiotctrl

- The repository wraps methods to control the following IoT devices in C and provides language bindings when
  needed:
  - LCUS-1 relay (`relay.c`/`relay.h`).
    <br />
    <img src="./assets/lcus-1_product-image.jpg" width="180" />
  - DL11B-MC temprature sensor (`temp-sensor.c`/`temp-sensor.h`).
    <br />
    <img src="./assets/dl11b-mc_product-image.jpg" width="180" />

## Device details

### LCUS-1 relay

- On Linux, you don't usually need any extra drivers if you see the following in `dmesg`:

```
# dmesg | grep tty
[    0.045680] print: console [tty0] enabled
[    1.025659] 00:00: ttyS0 at I/O 0x3f8 (irq = 4, base_baud = 115200) is a 16550A
[    1.532197] systemd[1]: Created slice system-getty.slice.
[    5.121271] usb 4-1: ch341-uart converter now attached to ttyUSB0
```

- Since "everything is a file", you can test if your device is working by directly writing bytes to the tty file:
  - Turn it on: `echo -n -e '\xA0\x01\x01\xA2' > /dev/ttyUSB0`
  - Turn it off: `echo -n -e '\xA0\x01\x00\xA1' > /dev/ttyUSB0`

### DL11B-MC temprature sensor

- The Chinese manual from its manufacturer is saved [here](./assets/dl11-mc_manual.pdf)

## Build and install

- Dependencies

  - `cmake` to generate `Makefile`: `apt install cmake`
  - `libmodbus` to provide support to Modbus RTU protocol: `apt install libmodbus-dev`
    - Can find its document [here](https://libmodbus.org/)

- Build

```
mkdir -p ./build
cd build
cmake ../
make
```

- Install

```
make install
```

### Node.js binding

- Node.js binding is provided for temp-sensor only.

- Dependencies

  - `node` (>= v14.18) and `npm` (>= 6.14)
  - `node-gyp` to build Node.js native addon: `npm install node-gyp`
  - Built `libiotctrl.so` according to [Build and install](#build-and-install)

- Build Node.js native addon

```
cd ./src/bindings/node
node-gyp configure
node-gyp build
```

- Test: `node temp_sensor.js`

### Python binding

- Python binding is provided for both temp-sensor and relay.

- Dependency:

  - Python 3.9
  - Built `libiotctrl.so` according to [Build and install](#build-and-install)

- Build

```
cd ./src/bindings/python
python3 ./iotctrl.py
```
