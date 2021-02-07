sort:
	gcc -Wall -o sort bubbleSort.c 

pot:
	arm-linux-gnueabihf-gcc -std=c99 -D _POSIX_C_SOURCE=200809L potMod.c -o potDriver
	cp i2c $(HOME)/cmpt433/public

i2c:
	arm-linux-gnueabihf-gcc -std=c99 -D _POSIX_C_SOURCE=200809L i2c.c -o i2c
	cp i2c $(HOME)/cmpt433/public

clean:
	rm i2c
