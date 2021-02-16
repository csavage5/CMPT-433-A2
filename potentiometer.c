#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "shutdownManager.h"

#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095

static pthread_t threadPipePID;
static pthread_mutex_t *pArrayLengthMutex;

int arrLength = 0;
FILE *fptr;

static int getVoltage0Reading();
static void* potentiometer_getLength(void *arg);
static void potentiometer_sendData();

// static void shutdownPipeThread();


// Start up thread
void potentiometer_init(pthread_mutex_t *ArrayLengthMutex) {
    pArrayLengthMutex = ArrayLengthMutex;
    pthread_create(&threadPipePID, NULL, potentiometer_getLength, NULL);
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
    while (true) {
        int reading = getVoltage0Reading();

        // prints to be removed later, just for testing
        if(reading < 500){
            // printf("Array will be of size: 1\n");
            arrLength = 1;
        }
        if(reading > 500 && reading < 1000){
            // printf("Array will be of size: 20\n");
            arrLength = 20;
        }
        if(reading > 1000 && reading < 1500){
            // printf("Array will be of size: 60\n");
            arrLength = 60;
        }
        if(reading > 1500 && reading < 2000){
            // printf("Array will be of size: 120\n");
            arrLength = 120;
        }
        if(reading > 2000 && reading < 2500){
            // printf("Array will be of size: 250\n");
            arrLength = 250;
        }
        if(reading > 2500 && reading < 3000){
            // printf("Array will be of size: 300\n");
            arrLength = 300;
        }
        if(reading > 3000 && reading < 3500){
            // printf("Array will be of size: 500\n");
            arrLength = 500;
        }
        if(reading > 3500 && reading < 4000){
            // printf("Array will be of size: 800\n");
            arrLength = 800;
        }
        if(reading > 4000 && reading < 4100){
            // printf("Array will be of size: 1200\n");
            arrLength = 1200;
        }
        if(reading == 4100){
            // printf("Array will be of size: 2100\n");
            arrLength = 2100;
        }
        if(current != arrLength){
            current = arrLength;
            // START OF CRITICAL SECTION
            pthread_mutex_lock(pArrayLengthMutex);
            potentiometer_sendData();
            pthread_mutex_unlock(pArrayLengthMutex);
            // END OF CRITICAL SECTION
        }
        
    }
    
    return NULL;
}

static void potentiometer_sendData() {
    fptr = fopen("lengthFile.txt", "w");
    fprintf(fptr, "%d", arrLength);
    fclose(fptr);

    printf("wrote value: %d\n", arrLength);
}

// static void shutdownPipeThread() {
//     printf("shutting down potentiometer pipe thread\n");
//     pthread_cancel(threadPipePID);
//     pthread_join(threadPipePID, NULL);
// }