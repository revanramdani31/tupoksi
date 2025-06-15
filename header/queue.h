#ifndef QUEUE_H
#define QUEUE_H

#include "utils.h"
#include "batch.h"

typedef struct QueueNode {
    void* data;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue {
    QueueNode* front;
    QueueNode* rear;
    int size;
} Queue;

// Basic queue operations
Queue* createQueue();
void enqueue(Queue* q, void* data);
void* dequeue(Queue* q);
int isQueueEmpty(Queue* q);
void freeQueue(Queue* q);
void* peekQueue(Queue* q);

// Batch-specific operations
void enqueueBatchItem(Queue* q, BatchDeleteItem* item);
BatchDeleteItem* dequeueBatchItem(Queue* q);
void freeQueueAndItems(Queue* q);

#endif // QUEUE_H 
