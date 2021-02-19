#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include "shutdownManager.h"
#include "displayDriver.h"

#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define TOGGLE_DISPLAY_1 "/sys/class/gpio/gpio61/value"
#define TOGGLE_DISPLAY_2 "/sys/class/gpio/gpio44/value"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"


static pthread_t threadPipePID;
static pthread_t threadDisplayPID;

static pthread_mutex_t mutDisplayValue = PTHREAD_MUTEX_INITIALIZER;

static void* displayThread(void *arg);
static void* pipeThread(void *arg);

static void displayVal(char display);
static void toggleLED(int LED, int state);
static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
// static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);

static char displayValue[3];
static int pipeFromArraySorter;
static int pipeValue;

// TODO take in arraySorter -> displayDriver 
void displayDriver_init(int *pipeToRead) {
    memset(displayValue, 0, sizeof(displayValue));
    pipeFromArraySorter = pipeToRead[0];
    // start up threads
    pthread_create(&threadPipePID, NULL, pipeThread, NULL);
    pthread_create(&threadDisplayPID, NULL, displayThread, NULL);
    printf("Module [displayDriver] initialized\n");

}

void displayDriver_shutdown() {
    pthread_cancel(threadDisplayPID);
    pthread_join(threadDisplayPID, NULL);
    
    pthread_cancel(threadPipePID);
    pthread_join(threadPipePID, NULL);

    printf("Module [displayDriver] shutting down...\n");

}

/* Threads */

/*
    Thread loop (pipe):
        wait for incoming pipe data
        if new data different from old data, lock & update *value

    Thread loop (display):
    - init display
    - loop until shutdown:
        if current value of character(s) is different than the one being displayed:
            - update digits to write
        - alternate displays to create illusion of two digits

*/


static void* displayThread(void *arg) {

    long seconds = 0;
    long nanoseconds = 5000000;
    struct timespec reqDelay = {seconds, nanoseconds};

    // initialize displays
    printf("Drive display (assumes GPIO #61 and #44 are output and 1)\n");
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    // Set display to output mode
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);
    close(i2cFileDesc);

    while(sm_isShutdown() == 0) {
        
        // lock *value here -don't want value changing mid-display cycle
        pthread_mutex_lock(&mutDisplayValue); 
        {
            // control first display
            toggleLED(1, 1);
            toggleLED(2, 0);
            displayVal(displayValue[0]);
            nanosleep(&reqDelay, (struct timespec *) NULL);

            // Control second display
            toggleLED(1, 0);
            toggleLED(2, 1);
            displayVal(displayValue[1]);
            nanosleep(&reqDelay, (struct timespec *) NULL);
        }
        pthread_mutex_unlock(&mutDisplayValue);
        
    }

    return NULL;

}

static void* pipeThread(void *arg) {
    // int lastValue = 0;

    while (sm_isShutdown() == 0) {
        // TODO wait for data from pipe
        read(pipeFromArraySorter, &pipeValue, sizeof(pipeValue));
        // TODO update *value when new pipe data comes in, assuming pipe won't send duplicate data back-to-back
        pthread_mutex_lock(&mutDisplayValue);
        if (pipeValue < 10) {
            sprintf(displayValue, "0%d", pipeValue);
        } 
        else if(pipeValue > 99) {
            sprintf(displayValue, "99");
        }
        else {
            sprintf(displayValue, "%d", pipeValue);
        }
        pthread_mutex_unlock(&mutDisplayValue); 
    }

    return NULL;           
}


static int initI2cBus(char* bus, int address){
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}

// Set value for a seg display LED
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value) {
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register.");
        exit(1);
    }
}

// static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr){
//     // To read a register, must first write the address
//     int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
//     if (res != sizeof(regAddr)) {
//         perror("I2C: Unable to write to i2c register.");
//         exit(1);
//     }

//     // Now read the value and return it
//     char value = 0;
//     res = read(i2cFileDesc, &value, sizeof(value));
//     if (res != sizeof(value)) {
//         perror("I2C: Unable to read from i2c register");
//         exit(1);
//     }
//     return value;
// }

static void displayVal(char display){

    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    if(display == '0'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0xa1);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x87);
    }
    if(display == '1'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0x01);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x80);
    }
    if(display == '2'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0x31);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x0e);
    }
    if(display == '3'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0xb0);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x06);
    }
    if(display == '4'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0x90);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x8a);
    }
    if(display == '5'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0xb0);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x8c);
    }
    if(display == '6'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0xb1);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x8c);
    }
    if(display == '7'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0x04);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x14);
    }
    if(display == '8'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0xb1);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x8f);
    }
    if(display == '9'){
        writeI2cReg(i2cFileDesc, REG_OUTA, 0x90);
        writeI2cReg(i2cFileDesc, REG_OUTB, 0x8f);
    }
    close(i2cFileDesc);
}

static void toggleLED(int LED, int state){
    if(LED == 1){
        FILE *pFile = fopen(TOGGLE_DISPLAY_1, "w");
        fprintf(pFile, "%d", state);
        fclose(pFile);
    }
    if( LED == 2){
        FILE *pFile = fopen(TOGGLE_DISPLAY_2, "w");
        fprintf(pFile, "%d", state);
        fclose(pFile);
    }
    
}


// void init() {
//     printf("Drive display (assumes GPIO #61 and #44 are output and 1)\n");
//     int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
//     // Set display to output mode
//     writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
//     writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);
//     close(i2cFileDesc);
// }

// int main() {
//     init();

//     // read from pipe here
    
//     display2(3);

//     return 0;
// }