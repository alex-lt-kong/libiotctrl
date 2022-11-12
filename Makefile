OPT = -O3 -Wall

main: relay.out relay.so temp-sensor.out temp-sensor.so

relay.out: relay.c
	gcc relay.c -o relay.out $(OPT)
relay.so: relay.c
	gcc -fPIC -shared -o relay.so relay.c $(OPT)
temp-sensor.out: temp-sensor.c
	gcc temp-sensor.c -o temp-sensor.out $(OPT) -lmodbus
temp-sensor.so: temp-sensor.c
	gcc -fPIC -shared -o temp-sensor.so $(OPT) -lmodbus