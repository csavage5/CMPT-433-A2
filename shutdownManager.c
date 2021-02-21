#include <pthread.h>
#include <semaphore.h>

#include "shutdownManager.h"

static pthread_cond_t condReleaseMain = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutReleaseMain = PTHREAD_MUTEX_INITIALIZER;
static int shutdownStatus = 0;

static pthread_mutex_t mutAccessShutdownStatus = PTHREAD_MUTEX_INITIALIZER;

// Returns 1 if shutdown has been initiated
int sm_isShutdown() {
    int temp = 0;
    pthread_mutex_lock(&mutAccessShutdownStatus);
    {
        temp = shutdownStatus;
    }
    pthread_mutex_unlock(&mutAccessShutdownStatus);
    
    return temp;
}

void sm_startShutdown() {
    pthread_mutex_lock(&mutAccessShutdownStatus);
    {
      shutdownStatus = 1;  
    }
    pthread_mutex_unlock(&mutAccessShutdownStatus);

    pthread_cond_signal(&condReleaseMain);

}

void sm_waitForShutdownOnMainThread() {
    // wait on condition variable until signal
    pthread_mutex_lock(&mutReleaseMain);
    {
        pthread_cond_wait(&condReleaseMain, &mutReleaseMain);
    }
    pthread_mutex_unlock(&mutReleaseMain);

}