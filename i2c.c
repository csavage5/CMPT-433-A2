#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
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

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr){
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
        perror("I2C: Unable to write to i2c register.");
        exit(1);
    }

    // Now read the value and return it
    char value = 0;
    res = read(i2cFileDesc, &value, sizeof(value));
    if (res != sizeof(value)) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }
    return value;
}

void displayVal(char display){

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

void toggleLED(int LED, int state){
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

void display2(int input){
    char value[3];
    if(input < 10) {
        sprintf(value, "0%d", input);
    }
    else {
        sprintf(value, "%d", input);
    }
    printf("Start of display2, first val: %c, second val: %c\n", value[0], value[1]);

    long seconds = 0;
    long nanoseconds = 5000000;
    struct timespec reqDelay = {seconds, nanoseconds};

    while(1){
        // control first LED
        toggleLED(1, 1);
        toggleLED(2, 0);
        displayVal(value[0]);
        nanosleep(&reqDelay, (struct timespec *) NULL);

        // Control second LED
        toggleLED(1, 0);
        toggleLED(2, 1);
        displayVal(value[1]);
        nanosleep(&reqDelay, (struct timespec *) NULL);
    }
    
}

void init() {
    printf("Drive display (assumes GPIO #61 and #44 are output and 1)\n");
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    // Set display to output mode
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);
    close(i2cFileDesc);
}

int main() {
    init();

    // read from pipe here
    
    display2(3);

    return 0;
}