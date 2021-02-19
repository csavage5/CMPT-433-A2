#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "shutdownManager.h"
#include "arraySorter.h"

static pthread_mutex_t mutMostRecentLength = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutCurrentArrayLength = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutArray = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutTotalSorts = PTHREAD_MUTEX_INITIALIZER;

//static pthread_mutex_t mutLength = PTHREAD_MUTEX_INITIALIZER;

int mostRecentLength = 100; // most recent length received from potentiometer
int currentArrayLength = 0; // length of current array being sorted
int current = 0;
long totalSorts = 0L;
int *array = NULL;
FILE *fptr;

static pthread_t threadPipePID;
static pthread_t threadSorterPID;
static pthread_t threadTimerPID;

static void* sorterThread(void *arg);
static void* pipeThread(void *arg);
static void* timerThread(void *arg);

static void createArray();
static void sort();
static void freeArray();

static void updateMostRecentLength(int newLength);
static int getMostRecentLength();
static int getLengthForNewArray();

static void incrementTotalSorts();

static void shutdownPipeThread();
static void shutdownSorterThread();
static void shutdownTimerThread();

static int pipeFromPot;
static int pipeToDisplay;

void arraySorter_init(int *pipeToRead, int *pipeToWrite) {
    //pArrayLengthMutex = ArrayLengthMutex;

    // save pipe info
    pipeFromPot = pipeToRead[0];
    pipeToDisplay = pipeToWrite[1];
    
    // start up threads
    pthread_create(&threadPipePID, NULL, pipeThread, NULL);
    pthread_create(&threadSorterPID, NULL, sorterThread, NULL);
    pthread_create(&threadTimerPID, NULL, timerThread, NULL);

    printf("Module [arraySorter] initialized\n");
}


/* Threads */

/*
    Thread loop (pipe):
        wait for incoming pipe data
        if new data different from old data, lock & overwrite mostRecentLength

    Thread loop (sorter):
        free array
        create array with current value of mostRecentLength
        sort array

        if timer <= 1 second:
            increment sorted counter
        else:
            send counter value to display module via pipe
            reset counter to 1
            reset timer to 0


        edge case: If it takes more than one second to sort a 
                   single array, then the second when an array 
                   finished sorting will show “1”, otherwise show “0’.

*/

// Sort arrays until shutdown
static void* sorterThread(void *arg) {
    printf("Thread [arraySorter]->sorterThread started\n");

    int localLength = 0;

    while (!sm_isShutdown()) {

        localLength = getLengthForNewArray();
        
        createArray(localLength);
        sort(localLength);

        incrementTotalSorts();

    }

    printf("Thread [arraySorter]->sorterThread starting shut down...\n");
    shutdownSorterThread();

    return NULL;
}

// Receive data via potentiometer pipe
static void* pipeThread(void *arg) {
    printf("Thread [arraySorter]->pipeThread started\n");

    char buffer[1024];

    while (!sm_isShutdown()) {   
        read(pipeFromPot, buffer, sizeof(buffer));
        printf("arraySorter read \"%s\" from incoming pipe\n", buffer);

        updateMostRecentLength(atoi(buffer));
 
    }
    printf("Thread [arraySorter]->pipeThread starting shut down...\n");
    shutdownPipeThread();

    return NULL;

}

// Record number of arrays sorted per second and send to displayDriver
static void* timerThread(void *arg) {
    printf("Thread [arraySorter]->timerThread started\n");

    time_t timeStart = time(NULL);
    long prevTotalSorts = arraySorter_getTotalSorts();
    int sortsInLastSecond = 0;

    while (!sm_isShutdown()) {

        if ( difftime(time(NULL), timeStart) >= 1 ){
            // CASE: second has elapsed

            // TODO send value over pipe to displayDriver
            sortsInLastSecond = arraySorter_getTotalSorts() - prevTotalSorts;
            printf("Sorted [%d] arrays in prev second\n", sortsInLastSecond);
            prevTotalSorts = arraySorter_getTotalSorts();
            write(pipeToDisplay, &sortsInLastSecond, sizeof(sortsInLastSecond));

            timeStart = time(NULL);
            
        }
    }
    printf("Thread [arraySorter]->timerThread starting shut down...\n");
    shutdownTimerThread();

    return NULL;

}


// Begin shutdown of sorter and pipe threads.
void arraySorter_shutdown() {

    shutdownPipeThread();
    shutdownSorterThread();
    shutdownTimerThread();

    // free heap memory
    freeArray();
    printf("Module [arraySorter] shut down\n");
}

static void shutdownPipeThread() {
    pthread_cancel(threadPipePID);
    pthread_join(threadPipePID, NULL);
    printf("Thread [arraySorter]->pipeThread shut down\n");
}

static void shutdownSorterThread() {
    pthread_cancel(threadSorterPID);
    pthread_join(threadSorterPID, NULL);
    printf("Thread [arraySorter]->sorterThread shut down\n");
}


static void shutdownTimerThread() {
    pthread_cancel(threadTimerPID);
    pthread_join(threadTimerPID, NULL);
    printf("Thread [arraySorter]->timerThread shut down\n");
}

/* Public Helper Functions */

// Get length of the array being sorted
int arraySorter_getSize() {
    int current = 0;

    // save new value into CurrentArrayLength
    pthread_mutex_lock(&mutCurrentArrayLength);
    current = currentArrayLength;
    pthread_mutex_unlock(&mutCurrentArrayLength);

    return current;
}

// Write the current array's contents into the given buffer
void arraySorter_getArray(char *buffer) {
    int length = arraySorter_getSize();
    char *cursor = buffer;

    //lock array, iterate through, write to buffer
    pthread_mutex_lock(&mutArray);

    for(int i = 0; i < length; i++) {

        if (i == length - 1) {
            // CASE: last element in array
            cursor += sprintf(cursor, "%d\n", array[i]);
        } else if ( (i+1) % 10 != 0 ) {
            // CASE: element not last on line
            cursor += sprintf(cursor, "%d, ", array[i]);
        } else {
            // CASE: element last on line
            cursor += sprintf(cursor, "%d,\n", array[i]);
        }
            
    }

    pthread_mutex_unlock(&mutArray);

}

// Returns an integer >= 1 corresponding to requested element, 0 if request is out of range
int arraySorter_getValue(int value) {
    
    pthread_mutex_lock(&mutArray);

    int currentLength = arraySorter_getSize();
    int foundValue = 0;

    if(value >= 1 && value <= currentLength) {    
        printf("value: %d\n", array[value-1]);
        foundValue = array[value-1];
    }

    pthread_mutex_unlock(&mutArray);

    return foundValue;

}


long arraySorter_getTotalSorts() {
    long temp;
    pthread_mutex_lock(&mutTotalSorts);
    temp = totalSorts;
    pthread_mutex_unlock(&mutTotalSorts);
    return temp;
}

static void incrementTotalSorts() {
    pthread_mutex_lock(&mutTotalSorts);
    totalSorts += 1L;
    pthread_mutex_unlock(&mutTotalSorts);
}



/* Private Helper Functions */

static void createArray(int length) {
    // initialize randomizer
    time_t t;
    srand((unsigned) time(&t));

    pthread_mutex_lock(&mutArray);

    free(array);
    array = NULL;

    // malloc space for array
    array = malloc(length * sizeof(int));

    // fill array with random integers from 1 to length
    for(int i = 0; i < length; i++){
        //from https://stackoverflow.com/questions/17846212/generate-a-random-number-between-1-and-10-in-c/49099642
        array[i] = rand() % (length-1) + 1;
    }

    pthread_mutex_unlock(&mutArray);
}

static void sort(int length) {
    int placeholder;
    int swapped = 1;
    while(swapped == 1){
        swapped = 0;
        for(int i = 0; i < length-1; i++) {
            
            pthread_mutex_lock(&mutArray);

            if(array[i] > array[i+1]) {
                placeholder = array[i];
                array[i] = array[i+1];
                array[i+1] = placeholder;
                swapped = 1;
            }

            pthread_mutex_unlock(&mutArray);

        }
    }
}

// Frees array - ONLY used after threads are shut down
static void freeArray() {
    free(array);
    array = NULL;
}

// save new value from the potentiometer
static void updateMostRecentLength(int newLength) {

     pthread_mutex_lock(&mutMostRecentLength);
     mostRecentLength = newLength;    
     pthread_mutex_unlock(&mutMostRecentLength);
   
}

// Get the most recent value from the potentiometer
static int getMostRecentLength() {
    int temp;
    pthread_mutex_lock(&mutMostRecentLength);
    {
        temp = mostRecentLength; 
    }
    pthread_mutex_unlock(&mutMostRecentLength);
   
    return temp;
}

// Get the most recent value from the potentiometer and save it to CurrentArrayLength
static int getLengthForNewArray() {
    int current = getMostRecentLength();

    // save new value into CurrentArrayLength
    pthread_mutex_lock(&mutCurrentArrayLength);
    {
        currentArrayLength = current;
    }
    pthread_mutex_unlock(&mutCurrentArrayLength);

    return current;
}

