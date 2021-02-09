#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


#include "commandListener.h"

#define MAX_LEN 1500  // 1500 bytes max in UDP packet
#define PORT 12345

static pthread_t threadPID;

static struct sockaddr_in sinLocal;
static unsigned int sin_len;
static int socketDescriptor;

static char *pMessage;
static char messageBuffer[MAX_LEN];
static char commands[2];

static void socketInit();
static void* listenerThread(void *arg);
void detectCommands();
void listenerShutdown();


void receiverInit() {

    //TODO start thread
    pthread_create(&threadPID, NULL, listenerThread, NULL);

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
        printf("Received %s", messageBuffer);

        if (messageLen == -1) {
            printf("Receive Error: %s\n", strerror(errno));
        }

        detectCommands();

        // TODO CASE: user sent "stop", call shutdown
        if (strcmp("stop", &commands[0]) == 0) {
            printf("Received command: shutdown");
            listenerShutdown();
            return NULL;
        }

        // TODO CASE: user sent "count"/"get"/"length"/"array", retrieve info from array module

        // TODO CASE: user sent "help", send help string
        if (strcmp("help", &commands[0]) == 0) {
            printf("Received command: help");
            strcpy(pMessage, "Commands: help, get, ...");
        }

        // reply with message
        int i = sendto(socketDescriptor, pMessage, strlen(pMessage), 
                        0, (struct sockaddr *) &sinRemote, sinRemote_len);
        
        if (i == -1) {
            printf("Reply Error: %s\n", strerror(errno));
        }

    }

    return NULL;

}

void detectCommands() {

    int i = 0;
    char *token = NULL;
    memset(commands, 0, sizeof(commands));

    while (i < 2 && (token = strtok(messageBuffer, " ")) != NULL ) {
        // will be MAX 2 tokens to the command
        commands[i] = *token;   
        i += 1;
    }
    
}

void listenerShutdown() {
    pthread_cancel(threadPID);
    pthread_join(threadPID, NULL);

    // close socket
    close(socketDescriptor);
    // free heap memory
    free(pMessage);
    pMessage = NULL;
}

int main() {
    receiverInit();
    int counter = 0;
    while(1){
        counter += 1;
    }   
    return 0;
}
