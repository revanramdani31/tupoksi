#include "queue.h"
#include <stdlib.h>

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) return NULL;
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue* q, void* data) {
    if (!q) return;
    
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) return;
    
    newNode->data = data;
    newNode->next = NULL;
    
    if (q->rear) {
        q->rear->next = newNode;
        q->rear = newNode;
    } else {
        q->front = q->rear = newNode;
    }
    q->size++;
}

void* dequeue(Queue* q) {
    if (!q || !q->front) return NULL;
    
    QueueNode* temp = q->front;
    void* data = temp->data;
    
    q->front = temp->next;
    if (!q->front) q->rear = NULL;
    
    free(temp);
    q->size--;
    return data;
}

int isQueueEmpty(Queue* q) {
    return !q || !q->front;
}

void enqueueBatchItem(Queue* q, BatchDeleteItem* item) {
    enqueue(q, item);
}

BatchDeleteItem* dequeueBatchItem(Queue* q) {
    return (BatchDeleteItem*)dequeue(q);
}

void freeQueue(Queue* q) {
    if (!q) return;
    
    while (!isQueueEmpty(q)) {
        void* data = dequeue(q);
        if (data) free(data);
    }
    free(q);
}

void freeQueueAndItems(Queue* q) {
    if (!q) return;
    
    while (!isQueueEmpty(q)) {
        BatchDeleteItem* item = dequeueBatchItem(q);
        if (item) free(item);
    }
    free(q);
} 