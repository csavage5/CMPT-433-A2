#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

#include "shutdownManager.h"

#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095

static pthread_t threadPipePID;
// static pthread_mutex_t *pArrayLengthMutex;

float arrLengthFloat = 0;
int arrLength = 0;

char *buffer;
FILE *fptr;

static int getVoltage0Reading();
static void* potentiometer_getLength(void *arg);
static void potentiometer_sendData();

int pipeToArraySorter;


// static void shutdownPipeThread();


// Start up thread
void potentiometer_init(int *pipeToWrite) {
    //pArrayLengthMutex = ArrayLengthMutex;
    
    // save pipe details
    //pipeToArraySorter = fdopen(pipeToWrite[1], "w");
    pipeToArraySorter = pipeToWrite[1];

    pthread_create(&threadPipePID, NULL, potentiometer_getLength, NULL);
    printf("Module [Potentionmeter] initialized\n");
}

// Read data from potentiometer on BBG
int getVoltage0Reading() {
    // Open file
    FILE *f = fopen(A2D_FILE_VOLTAGE0, "r");
    if (!f) {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf(" Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }
    // Get reading
    int a2dReading = 0;
    int itemsRead = fscanf(f, "%d", &a2dReading);
    if (itemsRead <= 0) {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }
    // Close file
    fclose(f);
    return a2dReading;
}


// Change reading from potentiometer to array length
static void* potentiometer_getLength(void *arg) {
    int current = 0;
    int arrayBounds[10] = {0,500,1000,1500,2000,2500,3000,3500,4000,4100};
    int arraySizes[10] = {1,20,60,120,250,300,500,800,1200,2100};
    while (true) {
        int reading = getVoltage0Reading();
        int i = 0;
        while(reading >= arrayBounds[i]){
            i++;
        }
        // Calculate array size
        int s = reading;
        int a = arrayBounds[i-1];
        int b = arrayBounds[i];
        int n = arraySizes[i-1];
        int m = arraySizes[i];
        double val1 = s-a;
        double val2 = b-a;
        arrLengthFloat = (((val1) / (val2)) * (m-n) + n);
        arrLength = arrLengthFloat;
        // printf("arrLengthfloat: %f, arrLength: %d\n", arrLengthFloat, arrLength);

        if(current != arrLength){
            current = arrLength;
            //arrLength = current;
            // START OF CRITICAL SECTION
            //pthread_mutex_lock(pArrayLengthMutex);
            potentiometer_sendData();
            //pthread_mutex_unlock(pArrayLengthMutex);
            // END OF CRITICAL SECTION
        }
        long seconds = 1;
        long nanoseconds = 0;
        struct timespec reqDelay = {seconds, nanoseconds};
        nanosleep(&reqDelay, (struct timespec *) NULL);
    }
    
    return NULL;
}

static void potentiometer_sendData() {
    fptr = fdopen(pipeToArraySorter, "w");
    
    buffer = (char *)malloc(5 * sizeof(char));
    memset(buffer, '\0', sizeof(*buffer));
    sprintf(buffer, "%d", arrLength);
    //snprintf(buffer, strlen(buffer), )

    write(pipeToArraySorter, buffer, sizeof(buffer));
    
    // fprintf(fptr, "%d", arrLength);
    // fflush(fptr);
    // close(pipeToArraySorter);

    printf("Potentiometer wrote value \"%d\" to pipe\n", buffer);

    free(buffer);
    buffer = NULL;
}

// static void shutdownPipeThread() {
//     printf("shutting down potentiometer pipe thread\n");
//     pthread_cancel(threadPipePID);
//     pthread_join(threadPipePID, NULL);
// }