#include <pthread.h>

#include "commandListener.h"
#include "shutdownManager.h"


int main() {

    // TODO call thread constructors
    commandListener_init();

    // TODO wait until shutdown is triggered 
    sm_waitForShutdownOnMainThread();

    // TODO call thread destructors
    commandListener_shutdown();


    return 0;
}