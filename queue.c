#include "queue.h"
#include "linkedlist.h"

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        perror("Gagal alokasi Queue");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    q->count = 0;
    return q;
}

int isQueueEmpty(Queue* q) {
    return q == NULL || q->front == NULL;
}

void enqueueBatchItem(Queue* q, BatchDeleteItem* item) {
    if(!q || !item) return;
    
    LinkedListNode* newNode = createLinkedListNode(item);
    if (!newNode) {
        free(item);
        perror("Gagal alokasi Node untuk Queue Batch");
        return;
    }
    
    newNode->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->count++;
}

BatchDeleteItem* dequeueBatchItem(Queue* q) {
    if (isQueueEmpty(q)) return NULL;
    
    LinkedListNode* temp = q->front;
    BatchDeleteItem* item = (BatchDeleteItem*)temp->data;
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    free(temp);
    q->count--;
    return item;
}

void freeQueueAndItems(Queue* q) {
    if (!q) return;
    
    while (!isQueueEmpty(q)) {
        BatchDeleteItem* item = dequeueBatchItem(q);
        if (item) free(item);
    }
    free(q);
}

void freeQueue(Queue* q) {
    if (q) {
        if (q->front != NULL) {
            LinkedListNode *current = q->front, *next_node;
            while(current != NULL) {
                next_node = current->next;
                free(current);
                current = next_node;
            }
        }
        free(q);
    }
} 