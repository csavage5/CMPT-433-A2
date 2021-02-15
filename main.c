#include <pthread.h>

#include "commandListener.h"
#include "shutdownManager.h"
#include "arraySorter.h"

int main() {

    // TODO call thread constructors
    commandListener_init();
    arraySorter_init();

    // TODO wait until shutdown is triggered 
    sm_waitForShutdownOnMainThread();

    // TODO call thread destructors
    commandListener_shutdown();
    arraySorter_shutdown();


    return 0;
}