#include <pthread.h>

#include "commandListener.h"
#include "shutdownManager.h"
#include "arraySorter.h"
#include "potentiometer.h"

static pthread_mutex_t ArrayLengthMutex;

int main() {

    // TODO call thread constructors
    commandListener_init();
    arraySorter_init(&ArrayLengthMutex);
    potentiometer_init(&ArrayLengthMutex);

    // TODO wait until shutdown is triggered 
    sm_waitForShutdownOnMainThread();

    // TODO call thread destructors
    commandListener_shutdown();
    arraySorter_shutdown();


    return 0;
}