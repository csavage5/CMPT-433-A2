sort:
	gcc -Wall -o sort bubbleSort.c 

pot:
	arm-linux-gnueabihf-gcc -std=c99 -D _POSIX_C_SOURCE=200809L potMod.c -o potDriver