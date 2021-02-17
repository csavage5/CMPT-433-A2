#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "shutdownManager.h"

static pthread_t threadPipePID;
static pthread_t threadSorterPID;

static pthread_mutex_t mutMostRecentLength = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutCurrentArrayLength = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutArray = PTHREAD_MUTEX_INITIALIZER;

//static pthread_mutex_t mutLength = PTHREAD_MUTEX_INITIALIZER;

int mostRecentLength = 100; // most recent length received from potentiometer
int currentArrayLength = 0; // length of current array being sorted
int current = 0;
int *array = NULL;
FILE *fptr;

static void* sorterThread(void *arg);
static void* pipeThread(void *arg);

static void createArray();
static void printArray();
static void sort();
static void freeArray();

static void updateMostRecentLength(int newLength);
static int getMostRecentLength();
static int getLengthForNewArray();

static void shutdownPipeThread();
static void shutdownSorterThread();

static int pipeFromPot;
//static FILE *pipeToDisplayDriver;

// TODO take in pipes
void arraySorter_init(int *pipeToRead) {
    //pArrayLengthMutex = ArrayLengthMutex;

    // save pipe info
    pipeFromPot = pipeToRead[0];
    
    // start up threads
    pthread_create(&threadPipePID, NULL, pipeThread, NULL);
    pthread_create(&threadSorterPID, NULL, sorterThread, NULL);
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
*/

static void* sorterThread(void *arg) {
    printf("starting to sort shit\n");

    int localLength = 0;

    while (!sm_isShutdown()) {
        freeArray();

        // get length for array
        localLength = getLengthForNewArray();
        
        createArray(localLength);
        sort(localLength);

        //TODO counter
    }
    
    shutdownSorterThread();

    return NULL;
}

static void* pipeThread(void *arg) {
    // TODO
    //int temp; // to keep busy loop busy
    printf("piping\n");
    char buffer[1024];

    while (!sm_isShutdown()) {   
        // READ FROM PIPE HERE
        //fptr = fdopen(pipeFromPot, "r");

        read(pipeFromPot, buffer, sizeof(buffer));
        printf("arraySorter read \"%s\" from incoming pipe\n", buffer);

        updateMostRecentLength(atoi(buffer));
 
    }

    shutdownPipeThread();

    return NULL;

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

// Write the current array into the given buffer
void arraySorter_getArray(char *buffer) {
    int length = arraySorter_getSize();

    //TODO lock array, iterate through
    pthread_mutex_lock(&mutArray);

    for(int i = 0; i < length; i++) {
        
        if (i == length - 1) {
            // CASE: last element in array
            sprintf(buffer, "%d\n", array[i]);
        } else if ( (i+1) % 10 != 0 ) {
            // CASE: element not last on line
            sprintf(buffer, "%d, ", array[i]);
        } else {
            // CASE: element last on line
            sprintf(buffer, "%d,\n", array[i]);
        }
            
    }

    pthread_mutex_unlock(&mutArray);

}

int arraySorter_getValue(int value) {
    if(value-1 < 0 || value > mostRecentLength) {
        printf("Error! Value out of index.\n");
    }
    else{
        printf("value: %d\n", array[value-1]);
        return array[value-1];
    }
    return -1;
}

void arraySorter_shutdown() {

    shutdownPipeThread();
    shutdownSorterThread();

    // free heap memory
    freeArray();
    printf("Module [arraySorter] shutting down...\n");
}


/* Private Helper Functions */

static void createArray(int length) {
    // initialize randomizer
    time_t t;
    srand((unsigned) time(&t));

    pthread_mutex_lock(&mutArray);

    // malloc space for array
    array = malloc(length * sizeof(int));

    // fill array with random integers from 0 to length
    for(int i = 0; i < length; i++){
        array[i] = rand() % length;
    }

    pthread_mutex_unlock(&mutArray);
}

// static void printArray(int length) {
//     for(int i = 0; i < length; i++) {
        
//         pthread_mutex_lock(&mutArray);
        
//         if(i != length-1){
//             printf("%d, ", array[i]);
//         }
//         else{
//             printf("%d\n", array[i]);
//         }

//         pthread_mutex_lock(&mutArray);

//     }
// }

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

static void freeArray() {
    free(array);
    array = NULL;
}

// save new value from the potentiometer
static void updateMostRecentLength(int newLength) {
    //TODO critical section locking

     pthread_mutex_lock(&mutMostRecentLength);
     mostRecentLength = newLength;    
     pthread_mutex_unlock(&mutMostRecentLength);
   
}

// Get the most recent value from the potentiometer
static int getMostRecentLength() {
    // TODO critical section locking
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

static void shutdownPipeThread() {
    pthread_cancel(threadPipePID);
    pthread_join(threadPipePID, NULL);
}

static void shutdownSorterThread() {
    pthread_cancel(threadSorterPID);
    pthread_join(threadSorterPID, NULL);
}

