#include <pthread.h>
#include <semaphore.h>

#include "shutdownManager.h"

static pthread_cond_t condReleaseMain = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutReleaseMain = PTHREAD_MUTEX_INITIALIZER;
static int shutdownStatus = 0;
// for reader-writer problem of checking for shutdown and starting shutdown
// reader-writer solution (sm_isShutdown and sm_startShutdown) adapted from https://arxiv.org/pdf/1309.4507.pdf
int numReaders = 0;
static pthread_mutex_t mutReaderIn;
static pthread_mutex_t mutReaderOut;

int writerRequest = 0;
static pthread_cond_t condWriterAccess = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutWriterAccess = PTHREAD_MUTEX_INITIALIZER;



int sm_isShutdown() {

    pthread_mutex_lock(&mutReaderIn);   // increment the reader counter
    {
        numReaders += 1;
    }
    pthread_mutex_unlock(&mutReaderIn);

    // *Critical Section*: check value of condition variable, return true / false
    int temp = shutdownStatus;
    
    pthread_mutex_lock(&mutReaderOut);
    {
        numReaders -= 1;

        if (writerRequest == 1 && numReaders == 0) {
            // CASE: writer wants to access and all readers
            //       have finished - give writer access

            pthread_cond_signal(&condWriterAccess);

        }

    }
    pthread_mutex_unlock(&mutReaderOut);


    return temp;
}

void sm_startShutdown() {
    // TODO release main thread from waiting
    pthread_cond_signal(&condReleaseMain);

    // update value of shutdownStatus boolean
    pthread_mutex_lock(&mutReaderIn);   // prevent new readers from entering critical section
    {
        pthread_mutex_lock(&mutReaderOut);  // lock for access to numReaders, writerRequest

        if (numReaders == 0) {
            // CASE: no active readers - can safely release mutReaderOut
            pthread_mutex_unlock(&mutReaderOut);
        } else {
            // CASE: active reader(s) blocked after or in critical section - release lock on reader exits and 
            //       wait for the last reader to signal 
            writerRequest = 1;
            pthread_mutex_unlock(&mutReaderOut);

            pthread_mutex_lock(&mutWriterAccess); 
            {
                // CASE: wait for all remaining readers to exit
                pthread_cond_wait(&condWriterAccess, &mutWriterAccess);
            }
            pthread_mutex_unlock(&mutWriterAccess); 

            // CASE: all readers are out of critical section(s), and no more can enter - 
            //       withdraw request 
            writerRequest = 0;
        }

        // *Critical section* - update shutdownStatus
        shutdownStatus = 1;
    }
    pthread_mutex_unlock(&mutReaderIn); // allow new readers in


}

void sm_waitForShutdownOnMainThread() {
    // wait on condition variable until signal
    pthread_mutex_lock(&mutReleaseMain);
    {
        pthread_cond_wait(&condReleaseMain, &mutReleaseMain);
    }
    pthread_mutex_unlock(&mutReleaseMain);

}