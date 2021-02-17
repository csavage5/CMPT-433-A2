#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "shutdownManager.h"

static pthread_t threadPipePID;
static pthread_t threadSorterPID;
//static pthread_mutex_t *pArrayLengthMutex;

//static pthread_mutex_t mutLength = PTHREAD_MUTEX_INITIALIZER;

int length = 100;
int current = 0;
int *array = NULL;
FILE *fptr;

static void* sorterThread(void *arg);
static void* pipeThread(void *arg);

static void createArray();
static void printArray();
static void sort();
static void freeArray();

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

static void* sorterThread(void *arg) {
    printf("starting to sort shit\n");
    while (!sm_isShutdown()) {
        freeArray();
        createArray();
        sort();


        //TODO counter
    }
    
    //shutdownSorterThread();

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

        // while (!feof(fptr) && !ferror(fptr) && fgets(buffer, sizeof(buffer), fptr) != NULL) {
        //     printf("arraySorter read \"%s\" from incoming pipe\n", buffer);
        // }
        //close(pipeFromPot);

        // update length function if recevied value is different from current
        // printf("in da loop\n");
        
        // char newVal[10];
        // int n = 10;
        
        // START OF CRITICAL SECTION
        // pthread_mutex_lock(pArrayLengthMutex);
        // fptr = fopen("lengthFile.txt", "r");
        // fgets(newVal, n, fptr);
        // fclose(fptr);
        // if(current != atoi(newVal)){
        //     printf("new array length: %s\n", newVal);
        //     current = atoi(newVal);
        // }
        // pthread_mutex_unlock(pArrayLengthMutex);
        // END OF CRITICAL SECTION
        //temp += 1;
    }

    //shutdownPipeThread();

    return NULL;

}



/* Public Helper Functions */


int arraySorter_getSize() {
    // TODO critical section locking
    
    return length;
}

int* arraySorter_getArray() {
    //TODO refactor to send back string of array data
    printArray();
    return array;
}

int arraySorter_getValue(int value) {
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
    printf("Module [arraySorter] shutting down...\n");
}


/* Private Helper Functions */

static void createArray() {
    // initialize randomizer
    time_t t;
    srand((unsigned) time(&t));

    if (current != length) {
        length = current;
    }

    // malloc space for array
    array = malloc(length * sizeof(int));

    // fill array with random integers from 0 to length
    for(int i = 0; i < length; i++){
        array[i] = rand() % length;
    }
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

// static void updateLength(int newLength) {
//     //TODO critical section locking
//     length = newLength;
// }

static void shutdownPipeThread() {
    pthread_cancel(threadPipePID);
    pthread_join(threadPipePID, NULL);
}

static void shutdownSorterThread() {
    pthread_cancel(threadSorterPID);
    pthread_join(threadSorterPID, NULL);
}

