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

static void socketInit();
static void* listenerThread(void *arg);


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
    bind(socketDescriptor, (struct sockaddr*) &sinLocal, sin_len);
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
    unsigned int sinRemote_len;

    static int messageLen; // tracks # of bytes received from packet, -1 if error


    pMessage = (char*)malloc(MAX_LEN * sizeof(char)); // malloc space for outgoing message
    memcpy(pMessage, messageBuffer, messageLen);
    strcpy(pMessage, "Hello there!\n");

    //while(!isShutdown()) {
    while(1) {

        // sinRemote captures counterparty address information
        messageLen = recvfrom(socketDescriptor, messageBuffer, MAX_LEN, 0, (struct sockaddr *) &sinRemote, &sinRemote_len);
        
        printf("Received %s", messageBuffer);

        // TODO CASE: user sent "stop", call shutdown

        // TODO CASE: user sent "count"/"get"/"length"/"array", retrieve info from array module

        // TODO CASE: user sent "help", send help string


        // reply with message
        int i = sendto(socketDescriptor, pMessage, strlen(pMessage), 
                        0, (struct sockaddr *) &sinRemote, sinRemote_len);
        
        if (i == -1) {
            printf("Socket Error: %s\n", strerror(errno));
        }

    }

    return NULL;

}

 void listenerShutdown(void) {
    pthread_cancel(threadPID);
    pthread_join(threadPID, NULL);

    // TODO close socket

    // TODO free heap memory
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
