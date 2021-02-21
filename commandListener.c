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

#define MAX_LEN_UDP 1500  // 1500 bytes max in UDP packet
#define PORT 12345

static pthread_t threadPID;

static struct sockaddr_in sinLocal;
static unsigned int sin_len;
static int socketDescriptor;

static struct sockaddr_in sinRemote;
static unsigned int sinRemote_len = sizeof(sinRemote);

static char *pMessage;
static char messageBuffer[MAX_LEN_UDP];
static char *commands[2];

static void socketInit();
static void* listenerThread(void *arg);
static void detectCommands();
static void sendReply();


void commandListener_init() {

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

    static int messageLen; // tracks # of bytes received from packet, -1 if error


    pMessage = (char*)malloc(16000 * sizeof(char)); // malloc space for receiving from getArray
    memcpy(pMessage, messageBuffer, messageLen);
    strcpy(pMessage, "Hello there!\n");

    char *endptr = NULL; // for strtol

    while(!sm_isShutdown()) {

        // sinRemote captures counterparty address information
        messageLen = recvfrom(socketDescriptor, messageBuffer, MAX_LEN_UDP, 0, (struct sockaddr *) &sinRemote, &sinRemote_len);
        printf("Received %s\n", messageBuffer);

        if (messageLen == -1) {
            printf("Receive Error: %s\n\n", strerror(errno));
        }

        detectCommands();
        printf("0: %s, 1: %s\n", commands[0], commands[1]);

        if (strcmp("stop", commands[0]) == 0) {
            // CASE: user sent "stop" - call shutdown
            printf("Received command: shutdown\n");
            strcpy(pMessage, "Program shutting down\n");

            sm_startShutdown();

        } else if ( strcmp("get", commands[0]) == 0) {
            // CASE: user sent "get" - check if second parameter is a number or command
            int userInput = (int) strtol(commands[1], &endptr, 10);

            if ( endptr != commands[1] ) {
                // CASE: user sent a number - get value from the array
                int value = arraySorter_getValue(userInput);
                
                if (value == 0) {
                    // CASE: userInput beyond range of array
                    sprintf(pMessage, "Error: parameter \"%d\" is out of range - enter a number between 1 and %d\n\n", userInput, arraySorter_getSize());
                } else {
                    sprintf(pMessage, "Value %d: %d\n\n", userInput, value);
                }

            } else {
                // CASE: user sent a string starting with a non-number
                if (strcmp("array", commands[1]) == 0) {
                    // CASE: user sent "array", send back contents of current array being sorted
                    arraySorter_getArray(pMessage);

                }  else if (strcmp("length", commands[1]) == 0) {
                    // CASE: user sent "length" - get length of current array being sorted
                    sprintf(pMessage, "Current array length: %d\n\n", arraySorter_getSize());

                } else {
                    sprintf(pMessage, "Error: parameter \"%s\" is invalid\n\n", commands[1]);
                }
            }

        } else if (strcmp("help", commands[0]) == 0) {
            printf("Received command: help\n");
            strcpy(pMessage, "Commands:\ncount: returns the number of arrays sorted so far\nget #: get the #-th element of the array currently being sorted\nget length: returns the length of the array currently being sorted\nget array: returns all data from the array currently being sorted\nstop: shutdown program\n\n");

        } else if (strcmp("count", commands[0]) == 0) {
            long temp = arraySorter_getTotalSorts();
            sprintf(pMessage, "Number of arrays sorted: %ld\n\n", temp);
        } else {
            sprintf(pMessage, "Error: invalid command \"%s\"\n\n", commands[0]);
        }

        printf("Size of pMessage: %u\n", strlen(pMessage));

        // reply with message
        sendReply();
        
        // wipe buffers for next command
        memset(messageBuffer, '\0', sizeof(messageBuffer));
        memset(pMessage, '\0', sizeof(*pMessage));
        //memcpy(pMessage, messageBuffer, messageLen);

    }

    printf("Thread [commandListener]->listenerThread starting shut down...\n");
    commandListener_shutdown();

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

static void sendReply() {

    // check if pMessage is bigger than MAX_LEN
    //  if it is, loop:
    //      - scan bytes, keeping pointer to last \n found
    //      - if you hit 1500 * iteration, send data from 'starting point' to
    //        the \n pointer

    char *pStart = pMessage;
    char *pEnd = NULL;      // last \n encountered
    char *pCursor = pMessage;

    int bytesSinceLastNewline = 0; //bytes since last \n
    int bytesToSend = 0;

    int err;
    //printf("pMessage is initially %d chars long\n", strlen(pCursor));
    while (strlen(pCursor) >= 1500) {

        for (int i = 0; i < 1500; i++) {
            
            bytesSinceLastNewline++;

            if (*pCursor == '\n') {
                pEnd = pCursor;
                bytesToSend += bytesSinceLastNewline;
                bytesSinceLastNewline = 0;
            }

            pCursor++;
            

        }

        printf("Sending %d bytes of pMessage...\n", bytesToSend);
        err = sendto(socketDescriptor, pStart, bytesToSend, 
                        0, (struct sockaddr *) &sinRemote, sinRemote_len);
       
        if (err == -1) {
            printf("Reply Error: %s\n\n", strerror(errno));
        }

        // set pStart, pCursor = pEnd + 1
        pStart = pEnd + 1;
        pCursor = pEnd + 1;
        bytesToSend = 0;
        bytesSinceLastNewline = 0;
    }

    printf("Sending %d bytes of pMessage...\n", strlen(pStart));
    err = sendto(socketDescriptor, pStart, strlen(pStart), 
                    0, (struct sockaddr *) &sinRemote, sinRemote_len);
    
    if (err == -1) {
        printf("Reply Error: %s\n\n", strerror(errno));
    }

}


void commandListener_shutdown() {
    pthread_cancel(threadPID);
    pthread_join(threadPID, NULL);

    printf("Thread [commandListener]->listenerThread shut down\n");

    // close socket
    close(socketDescriptor);

    // free heap memory
    free(pMessage);
    pMessage = NULL;

    printf("Module [commandListener] shut down\n");
}
