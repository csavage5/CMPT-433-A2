#include <pthread.h>
#include <unistd.h>

#include "commandListener.h"
#include "shutdownManager.h"
#include "arraySorter.h"
#include "potentiometer.h"

//static pthread_mutex_t ArrayLengthMutex;

int main() {

    // Create pipes
    int pipePotToArraySorter[2];
    pipe(pipePotToArraySorter);

    // TODO call thread constructors
    commandListener_init();
    arraySorter_init(&pipePotToArraySorter);
    potentiometer_init(&pipePotToArraySorter);

    // TODO wait until shutdown is triggered 
    sm_waitForShutdownOnMainThread();

    // TODO call thread destructors
    commandListener_shutdown();
    arraySorter_shutdown();
    potentiometer_shutdown();


    return 0;
}