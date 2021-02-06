#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095

int getVoltage0Reading() {
// Open file
FILE *f = fopen(A2D_FILE_VOLTAGE0, "r");
if (!f) {
printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
printf(" Check /boot/uEnv.txt for correct options.\n");
exit(-1);
}
// Get reading
int a2dReading = 0;
int itemsRead = fscanf(f, "%d", &a2dReading);
if (itemsRead <= 0) {
printf("ERROR: Unable to read values from voltage input file.\n");
exit(-1);
}
// Close file
fclose(f);
return a2dReading;
}

int main() {
    int change = 0;
    while (true) {
        int reading = getVoltage0Reading();
        int length = 0;

        // prints to be removed later, just for testing
        if(reading < 500){
            printf("Array will be of size: 1\n");
            length = 1;
        }
        else if(reading > 500 && reading < 1000){
            printf("Array will be of size: 20\n");
            length = 20;
        }
        else if(reading > 1000 && reading < 1500){
            printf("Array will be of size: 60\n");
            length = 60;
        }
        else if(reading > 1500 && reading < 2000){
            printf("Array will be of size: 120\n");
            length = 120;
        }
        else if(reading > 2000 && reading < 2500){
            printf("Array will be of size: 250\n");
            length = 250;
        }
        else if(reading > 2500 && reading < 3000){
            printf("Array will be of size: 300\n");
            length = 300;
        }
        else if(reading > 3000 && reading < 3500){
            printf("Array will be of size: 500\n");
            length = 500;
        }
        else if(reading > 3500 && reading < 4000){
            printf("Array will be of size: 800\n");
            length = 800;
        }
        else if(reading > 4000 && reading < 4100){
            printf("Array will be of size: 1200\n");
            length = 1200;
        }
        else if(reading == 4100){
            printf("Array will be of size: 2100\n");
            length = 2100;
        }
        
        // to be removed, just for testing
        sleep(0.5);
        if(change != length && length != 0){
            printf("new length of the array is: %d\n", length);
            change = length;
        }
        // send length to sorting thread here
    }
    return 0;
}