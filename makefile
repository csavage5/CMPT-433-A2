sort:
	gcc -Wall -o sort bubbleSort.c 

pot:
	arm-linux-gnueabihf-gcc -std=c99 -D _POSIX_C_SOURCE=200809L potMod.c -o potDriver

listener:
	arm-linux-gnueabihf-gcc -std=c99 -D _POSIX_C_SOURCE=200809L commandListener.c -o listener -pthread
	cp listener $(HOME)/cmpt433/public/myApps/

