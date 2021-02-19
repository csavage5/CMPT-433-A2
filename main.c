#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "commandListener.h"
#include "shutdownManager.h"
#include "arraySorter.h"
#include "potentiometer.h"
#include "displayDriver.h"


// Shellscript to set pins to i2c
#define SHELLSCRIPT "\
#/bin/bash \n\
sleep 1 \n\
echo out > /sys/class/gpio/gpio61/direction \n\
echo out > /sys/class/gpio/gpio44/direction \n\
sleep 1 \n\
config-pin P9_18 i2c \n\
config-pin P9_17 i2c \n\
"


int main() {

    // Create pipes
    int pipePotToArraySorter[2];
    pipe(pipePotToArraySorter);
    int pipeArraySorterToDisplay[2];
    pipe(pipeArraySorterToDisplay);

    // TODO call thread constructors
    system(SHELLSCRIPT);
    commandListener_init();
    arraySorter_init(&pipePotToArraySorter, &pipeArraySorterToDisplay);
    potentiometer_init(&pipePotToArraySorter);
    displayDriver_init(&pipeArraySorterToDisplay);

    // TODO wait until shutdown is triggered 
    sm_waitForShutdownOnMainThread();

    // TODO call thread destructors
    commandListener_shutdown();
    arraySorter_shutdown();
    potentiometer_shutdown();
    displayDriver_shutdown();


    return 0;
}