#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

#include "shutdownManager.h"
#include "potentiometer.h"

#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095

static pthread_t threadPipePID;

static float arrLengthFloat = 0;
static int arrLength = 0;

static char buffer[5];

static int getVoltage0Reading();
static void* potentiometer_getLength(void *arg);
static void potentiometer_sendData();

static int pipeToArraySorter;


// Start up thread
void potentiometer_init(int *pipeToWrite) {
    
    // save pipe details
    pipeToArraySorter = pipeToWrite[1];

    pthread_create(&threadPipePID, NULL, potentiometer_getLength, NULL);
    printf("Module [potentiometerReader] initialized\n");
}

// Read data from potentiometer on BBG
static int getVoltage0Reading() {
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
    while (!sm_isShutdown()) {
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

        if(current != arrLength){
            current = arrLength;
            potentiometer_sendData();
        }
        long seconds = 1;
        long nanoseconds = 0;
        struct timespec reqDelay = {seconds, nanoseconds};
        nanosleep(&reqDelay, (struct timespec *) NULL);
    }
    printf("Thread [potentiometerReader]->getLength starting shut down...\n");

    potentiometer_shutdown();

    return NULL;
}

// Function to send length of arrays to sorting module
static void potentiometer_sendData() {
    memset(buffer, '\0', sizeof(*buffer));
    sprintf(buffer, "%d", arrLength);
    write(pipeToArraySorter, buffer, sizeof(buffer));

    printf("Potentiometer wrote value \"%s\" to pipe\n", buffer);
}

void potentiometer_shutdown() {
    pthread_cancel(threadPipePID);
    pthread_join(threadPipePID, NULL);

    // close pipes
    close(pipeToArraySorter);

    printf("Thread [potentiometerReader]->getLength shut down\n");
    printf("Module [potentiometerReader] shut down\n");
}