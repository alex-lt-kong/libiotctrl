const temp_sensors = require('./temp_sensor.node');

console.log(temp_sensors.get_temperature("/dev/ttyUSB1", 0));  // base32("Hello world!")