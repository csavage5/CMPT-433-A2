
#include "commandListener.h"
#include <pthread.h>

int main() {

    // TODO call thread constructors
    commandListener_init();

    // TODO wait until shutdown is triggered 


    // TODO call thread destructors
    commandListener_shutdown();


    return 0;
}