# sort:
# 	gcc -Wall -o sort bubbleSort.c 

pot:
	arm-linux-gnueabihf-gcc -std=c99 -D _POSIX_C_SOURCE=200809L potMod.c -o potDriver
	cp i2c $(HOME)/cmpt433/public

i2c:
	arm-linux-gnueabihf-gcc -std=c99 -D _POSIX_C_SOURCE=200809L i2c.c -o i2c
	cp i2c $(HOME)/cmpt433/public

clean:
	rm i2c

sorter:
	arm-linux-gnueabihf-gcc -g -Wall -Werror -std=c99 -D _POSIX_C_SOURCE=200809L main.c shutdownManager.c commandListener.c arraySorter.c potentiometer.c displayDriver.c -o sorter -pthread
	cp sorter $(HOME)/cmpt433/public/myApps/

sorter-clean:
	rm sorter

