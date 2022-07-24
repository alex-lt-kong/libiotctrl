main: rc.c
	gcc rc.c -o rc.out
	gcc -fPIC -shared -O3 -o rc.so rc.c