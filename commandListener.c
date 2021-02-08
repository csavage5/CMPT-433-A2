#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "commandListener.h"

#define MAX_LEN 1500  // 1500 bytes max in UDP packet
#define PORT 12345


static struct sockaddr_in sin;
static int socketDescriptor;

void receiverInit() {
    
    init_socket();

    //TODO start thread
    pthread_create(&threadPID, NULL, receiverThread, NULL);

}


static void init_socket() {

    // initialize sockets
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    // create and bind to socket
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));
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


void* receiverThread(void *arg) {
    
    while(!isShutdown() && !termCharEntered) {
        
        messageLen = recvfrom(s_socketDescriptor, localBuffer, MSG_MAX_LEN, 0 , (struct sockaddr *) s_sinRemote, &s_sin_len);
        
        messageReceived = (char*)malloc(MSG_MAX_LEN*sizeof(char)); // malloc space for next message
        memcpy(messageReceived, localBuffer, messageLen);

        // TODO CASE: user sent STOP, call shutdown

        // TODO CASE: user sent "count"/"get"/"length"/"array", retrieve info from array module

        // TODO CASE: user sent "help", send help string

        // TODO get counterparty address info from message

        // reply with message
        int i = sendto(s_socketDescriptor, messageToSend, strlen(messageToSend), 
                        0, (struct sockaddr *) s_sinRemote, s_sin_len);
        
        if (i == -1){
            printf("Socket Error: %s\n", strerror(errno));
        }

    }

    return NULL;

}

