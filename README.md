# USB IoT device control

* The repository wraps methods to control the following USB IoT devices:
  * LCUS-1 relay.
    <img src="./assets/lcus-1_product-image.jpg" width="360" />
  * DL11B-MC temprature sensor.
    <img src="./assets/dl11b-mc_product-image.jpg" width="360" />


## LCUS-1 relay

* On Linux, you don't usually need any extra drivers if you see the following in `dmesg`:

```
# dmesg | grep tty
[    0.045680] printk: console [tty0] enabled
[    1.025659] 00:00: ttyS0 at I/O 0x3f8 (irq = 4, base_baud = 115200) is a 16550A
[    1.532197] systemd[1]: Created slice system-getty.slice.
[    5.121271] usb 4-1: ch341-uart converter now attached to ttyUSB0
```

* Since "everything is a file", you can test if your device is working by directly writing bytes to the tty file: 
  * Turn it on:  `echo -n -e '\xA0\x01\x01\xA2' > /dev/ttyUSB0`
  * Turn it off: `echo -n -e '\xA0\x01\x00\xA1' > /dev/ttyUSB0`

## DL11B-MC temprature sensor

* `apt install libmodbus-dev`: provide support to Modbus RTU protocol.
  * Can find its document [here](https://libmodbus.org/)

* DL11B's Chinese manual from its manufacturer is saved [here](./assets/dl11-mc_manual.pdf)