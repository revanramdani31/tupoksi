#ifndef QUEUE_H
#define QUEUE_H

#include "utils.h"

// Queue structure
struct Queue {
    LinkedListNode* front;
    LinkedListNode* rear;
    int count;
};

// Function prototypes
Queue* createQueue();
int isQueueEmpty(Queue* q);
void enqueueBatchItem(Queue* q, BatchDeleteItem* item);
BatchDeleteItem* dequeueBatchItem(Queue* q);
void freeQueueAndItems(Queue* q);
void freeQueue(Queue* q);

#endif // QUEUE_H 