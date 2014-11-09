#include "cs537.h"
#include "request.h"
#include "stdio.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

//buffer variables
int *buffer;
int bufHead;
int bufTail;
int bufSize;
int bufCount;

pthread_cond_t notFull = PTHREAD_COND_INITIALIZER; 
pthread_cond_t doRequest = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// CS537: Parse the new arguments too
void getargs(int *port, int *threads, int *buffers, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threads> <bufferlen>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *buffers = atoi(argv[3]);
}

/**
 * This is actually a enqueue function, item will be put to the end of the queue
 */
int addRequest(int requestFD){
    if(bufCount == bufSize){
        return -1;
    } else {
        buffer[bufTail] = requestFD;//put fd in queue
        bufTail = (bufTail + 1)%bufSize; //inc tail
        bufCount++; //inc # items in buf
        printf("added fd %d to buffer.\n", requestFD);
    }
    return 0;
}

/**
 * This is the dequeue function. Items will be removed from the head of the queue
 */
int getNextRequest(){
    int requestFD;
    if(bufCount == 0)
        return -1; //buffer empty, return -1
    else{
        requestFD = buffer[bufHead]; //grab next item
        buffer[bufHead] = -1; //clear the slot
        bufCount--; //dec # items
        bufHead = (bufHead + 1)%bufSize; //increment the bufferhead
    }
    return requestFD;

}

//Consumer, for processing requests from buffer
void* consumer(void* arg){
    while(1){
        pthread_mutex_lock(&mutex);
        while(bufCount == 0){//if there is no request in the queue, make the thread wait
            pthread_cond_wait(&doRequest, &mutex);
        }
        int requestFD = getNextRequest();
        printf("fd %d grabbed from buffer.\n", requestFD);
        pthread_cond_signal(&notFull); //tell main we are not full
        pthread_mutex_unlock(&mutex);//unlock
        requestHandle(requestFD);//do request
        Close(requestFD); //close the request
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads, &bufSize, argc, argv);

    //initialize buffer
    buffer = (int*)malloc(bufSize*sizeof(int));
    bufCount = 0;
    bufHead = 0;
    bufTail = 0;

    //create threads
    pthread_t *threadPool = (pthread_t *)malloc(threads*sizeof(pthread_t));

    //initiate threads
    int i;
    for(i = 0; i < threads; i++){
        int error = pthread_create(&threadPool[i], NULL, consumer, (void*)NULL);
        if(error != 0){
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(0);
        }
    }

    listenfd = Open_listenfd(port);
    while (1) {
    	clientlen = sizeof(clientaddr);
    	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        // main thread producer section
        pthread_mutex_lock(&mutex);
        while(bufCount == bufSize){//if the queue is full, make the thread wait
            pthread_cond_wait(&notFull,&mutex);
        }
        addRequest(connfd); //add connection fd to buffer
        pthread_cond_signal(&doRequest); //tell consumers to do request
        pthread_mutex_unlock(&mutex); //give up the mutex
    }
}


    


 
