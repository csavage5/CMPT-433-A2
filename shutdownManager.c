#include <pthread.h>

#include "shutdownManager.h"

static pthread_cond_t condEndProgram = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutEndProgram = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t mutCheckStatus_isShutdown = PTHREAD_MUTEX_INITIALIZER;
static int isShutdown = 0;

int sm_isShutdown() {
    // check value of condition variable, return true / false
    int temp = 0;
    pthread_mutex_lock(&mutCheckStatus_isShutdown); 
    {
        temp = isShutdown;
    }
    pthread_mutex_unlock(&mutCheckStatus_isShutdown);
    
    return temp;
}

void sm_startShutdown() {
    // TODO release main thread from waiting
    pthread_cond_signal(&condEndProgram);
}

void sm_waitForShutdownOnMainThread() {
    // wait on condition variable until signal
    pthread_mutex_lock(&mutEndProgram);
    {
        pthread_cond_wait(&condEndProgram, &mutEndProgram);
    }
    pthread_mutex_unlock(&mutEndProgram);

}