all:
	arm-linux-gnueabihf-gcc -g -Wall -Werror -std=c99 -D _POSIX_C_SOURCE=200809L main.c shutdownManager.c commandListener.c arraySorter.c potentiometer.c displayDriver.c -o sorter -pthread
	cp sorter $(HOME)/cmpt433/public/myApps/
	arm-linux-gnueabihf-gcc -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror noworky.c -o noworky
	cp noworky $(HOME)/cmpt433/public/myApps/

clean:
	rm sorter noworky

