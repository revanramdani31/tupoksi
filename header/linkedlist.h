#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "utils.h"

typedef struct LinkedListNode {
    void* data;
    struct LinkedListNode* next;
} LinkedListNode;

// Function prototypes
LinkedListNode* createLinkedListNode(void* data);
void appendToList(LinkedListNode** head, void* data);
void removeFromList(LinkedListNode** head, void* data_to_remove);

#endif // LINKEDLIST_H 