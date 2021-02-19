#include <pthread.h>
#include <unistd.h>

#include "commandListener.h"
#include "shutdownManager.h"
#include "arraySorter.h"
#include "potentiometer.h"
#include "displayDriver.h"


int main() {

    // Create pipes
    int pipePotToArraySorter[2];
    pipe(pipePotToArraySorter);
    int pipeArraySorterToDisplay[2];
    pipe(pipeArraySorterToDisplay);

    // TODO call thread constructors
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