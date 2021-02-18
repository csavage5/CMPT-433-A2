#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


#include "commandListener.h"
#include "shutdownManager.h"
#include "arraySorter.h"

#define MAX_LEN 1500  // 1500 bytes max in UDP packet
#define PORT 12345

static pthread_t threadPID;

static struct sockaddr_in sinLocal;
static unsigned int sin_len;
static int socketDescriptor;

static char *pMessage;
static char messageBuffer[MAX_LEN];
static char *commands[2];

static void socketInit();
static void* listenerThread(void *arg);
static void detectCommands();


void commandListener_init() {

    //TODO start thread
    pthread_create(&threadPID, NULL, listenerThread, NULL);
    printf("Module [commandListener] initialized\n");

}


static void socketInit() {

    // initialize sockets
    memset(&sinLocal, 0, sizeof(sinLocal));
    sinLocal.sin_family = AF_INET;
    sinLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    sinLocal.sin_port = htons(PORT);
    sin_len = sizeof(sinLocal);

    // create and bind to socket
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    int err = bind(socketDescriptor, (struct sockaddr*) &sinLocal, sin_len);

    if (err == -1) {
        printf("Bind Error: %s\n", strerror(errno));
    }
}


/*
    Thread loop:
        wait for incoming command
        received command:
            if count/get/length/array, pull from other module
            if help, return pre-made summary
            if shutdown, call shutdown command
        reply to message with response

*/


static void* listenerThread(void *arg) {

    socketInit();

    struct sockaddr_in sinRemote;
    unsigned int sinRemote_len = sizeof(sinRemote);

    static int messageLen; // tracks # of bytes received from packet, -1 if error


    pMessage = (char*)malloc(MAX_LEN * sizeof(char)); // malloc space for outgoing message
    memcpy(pMessage, messageBuffer, messageLen);
    strcpy(pMessage, "Hello there!\n");

    //while(!isShutdown()) {
    while(1) {

        // sinRemote captures counterparty address information
        messageLen = recvfrom(socketDescriptor, messageBuffer, MAX_LEN, 0, (struct sockaddr *) &sinRemote, &sinRemote_len);
        printf("Received %s\n", messageBuffer);

        if (messageLen == -1) {
            printf("Receive Error: %s\n", strerror(errno));
        }

        detectCommands();
        printf("0: %s, 1: %s\n", commands[0], commands[1]);

        if (strcmp("stop", commands[0]) == 0) {
            // CASE: user sent "stop" - call shutdown
            printf("Received command: shutdown\n");
            sm_startShutdown();
            commandListener_shutdown();

        } else if ( strcmp("get", commands[0]) == 0) {
            // CASE: user sent "get" - check if second parameter is a number or command
            int userInput = atoi(commands[1]);
            
            if ( userInput > 0 ) {
                // CASE: user sent a number - get value from the array
                int value = arraySorter_getValue(userInput);
                
                if (value == 0) {
                    // CASE: userInput beyond range of array
                    sprintf(pMessage, "Error: parameter %d is out of range\n", userInput);
                } else {
                    sprintf(pMessage, "Value %d: %d\n", userInput, value);
                }

            } else {
                // CASE: user sent a string containing at least one non-numeral character
                if (strcmp("array", commands[1]) == 0) {
                    // CASE: user sent "array", send back contents of current array being sorted
                    arraySorter_getArray(pMessage);

                }  else if (strcmp("length", commands[1]) == 0) {
                    // CASE: user sent "length" - get length of current array being sorted
                    sprintf(pMessage, "Current array length: %d\n", arraySorter_getSize());

                } else {
                    sprintf(pMessage, "Error: parameter %s is invalid\n", commands[1]);
                }
            }

        } else if (strcmp("help", commands[0]) == 0) {
            // TODO CASE: user sent "help", send help string
            printf("Received command: help\n");
            strcpy(pMessage, "Commands: help, get, ...\n");

        } else if (strcmp("count", commands[0]) == 0) {
            long temp = arraySorter_getTotalSorts();
            sprintf(pMessage, "Number of arrays sorted: %ld\n", temp);
        } else {
            strcpy(pMessage, "Error: invalid command\n");
        }

        // reply with message
        int i = sendto(socketDescriptor, pMessage, strlen(pMessage), 
                        0, (struct sockaddr *) &sinRemote, sinRemote_len);
        
        if (i == -1) {
            printf("Reply Error: %s\n", strerror(errno));
        }

        // wipe messageBuffer for next command
        memcpy(pMessage, messageBuffer, messageLen);

    }

    return NULL;

}

static void detectCommands() {

    int i = 0;
    char *newline = 0;
    char *token = NULL;
    commands[0] = NULL;
    commands[1] = NULL;
    
    token = strtok(messageBuffer, " ");
   
    while (i < 2 && token != NULL ) {
        // will be MAX 2 tokens to the command
        
        // CASE: token contains a newline character - remove it
        // adapted from https://stackoverflow.com/questions/9628637/how-can-i-get-rid-of-n-from-string-in-c
        if ( (newline = strchr(token, '\n')) != NULL) {
            *newline = '\0';
        }
        commands[i] = token;   
        token = strtok(NULL, " ");
        i += 1;
    }
    
}

void commandListener_shutdown() {
    pthread_cancel(threadPID);
    pthread_join(threadPID, NULL);

    // close socket
    close(socketDescriptor);

    // free heap memory
    free(pMessage);
    pMessage = NULL;
    printf("Module [commandListener] shutting down...\n");
}
