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


// void* receiverThread(void *arg) {
    
//     while(!isShutdown() && !termCharEntered) {
        
//         messageLen = recvfrom(s_socketDescriptor, localBuffer, MSG_MAX_LEN, 0 , (struct sockaddr *) s_sinRemote, &s_sin_len);
        
//         messageReceived = (char*)malloc(MSG_MAX_LEN*sizeof(char)); // malloc space for next message
//         memcpy(messageReceived, localBuffer, messageLen);

//         // CASE: other user entered termination character, stop receiving data
//         if (checkForTerminationChar(messageReceived)) {
//             termCharEntered = true;
//         }

//         // CASE: user did not enter termination character - enqueue message for Sender
//         addToIncomingMessageBuffer(messageReceived); 
//         // once enqueued for output, let it handle freeing the message
//         messageReceived = NULL;
//     }

//     return NULL;

// }


// void addToIncomingMessageBuffer(char* message) {
//     pthread_mutex_lock(pMutexBufferModify);
//     {
//         while (List_prepend(pMessageBuffer, message) == -1) {
//             //CASE: buffer is full - block this thread until Output 
//             //      removes an item and signals condition variable
//             pthread_cond_wait(pCondBufferFull, pMutexBufferModify);
//         }

//         // CASE: Output thread may be blocked waiting for populated
//         //       buffer - signal wake up
//         pthread_cond_signal(pCondBufferEmpty);
//     }
//     pthread_mutex_unlock(pMutexBufferModify);
// }


// void receiverShutdown(void) {
//     pthread_cancel(threadPID);
//     pthread_join(threadPID, NULL);

//     // TODO free heap memory
//     free(messageReceived);
//     messageReceived = NULL;
// }
