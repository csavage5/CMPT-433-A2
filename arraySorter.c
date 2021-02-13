#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "shutdownManager.h"

static pthread_t threadPipePID;
static pthread_t threadSorterPID;

static pthread_mutex_t mutLength = PTHREAD_MUTEX_INITIALIZER;

int length = 100;
int *array = NULL;

static void createArray();
static void printArray();
static void sort();
static void freeArray();

static void shutdownPipeThread();
static void shutdownSorterThread();


/* Public Functions */

// TODO take in pipes
void arraySorter_init() {
    // start up threads
    pthread_create(&threadPipePID, NULL, sorterThread, NULL);
    pthread_create(&threadSorterPID, NULL, sorterThread, NULL);
}


/*
    Thread loop (pipe):
        wait for incoming pipe data
        if new data different from old data, lock & overwrite length

    Thread loop (sorter):
        free array
        create array with current value of length
        sort array

        if timer <= 1 second:
            increment sorted counter
        else:
            send counter value to display module via pipe
            reset counter to 1
            reset timer to 0
*/


static void sorterThread() {
    
    while (!sm_isShutdown()) {
        freeArray();
        createArray();
        sort();


        //TODO counter
    }
    
    shutdownSorterThread();
}

static void pipeThread() {
    // TODO

    while (sm_isShutdown()) {

    }

    shutdownPipeThread();

}


static int arraySorter_getSize() {
    // TODO critical section locking
    
    return length;
}

static void arraySorter_getArray() {
    printArray();
    return array;
}

static int arraySorter_getValue(int value) {
    if(value-1 < 0 || value > length) {
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
    printf("Module [commandListener] shutting down...\n");
}


/* Helper Functions */

static void createArray() {
    // initialize randomizer
    time_t t;
    srand((unsigned) time(&t));

    // malloc space for array
    array = malloc(length * sizeof(int));

    // fill array with random integers from 0 to length
    for(int i = 0; i < length; i++){
        array[i] = rand() % length;
    }
    return array;
}

static void printArray() {
    for(int i = 0; i < length; i++) {
        if(i != length-1){
            printf("%d, ", array[i]);
        }
        else{
            printf("%d\n", array[i]);
        }
    }
}

static void sort() {
    int placeholder;
    int swapped = 1;
    while(swapped == 1){
        swapped = 0;
        for(int i = 0; i < length-1; i++) {
            if(array[i] > array[i+1]) {
                placeholder = array[i];
                array[i] = array[i+1];
                array[i+1] = placeholder;
                swapped = 1;
            }
        }
    }
}

static void freeArray() {
    free(array);
    array = NULL;
}

static void updateLength(int newLength) {
    //TODO critical section locking
    length = newLength;
}

static void shutdownPipeThread() {
    pthread_cancel(threadPipePID);
    pthread_join(threadPipePID, NULL);
}

static void shutdownSorterThread() {
    pthread_cancel(threadSorterPID);
    pthread_join(threadSorterPID, NULL);
}

