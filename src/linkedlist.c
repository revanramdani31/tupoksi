#include "linkedlist.h"

LinkedListNode* createLinkedListNode(void* data) {
    LinkedListNode* newNode = (LinkedListNode*)malloc(sizeof(LinkedListNode));
    if (!newNode) {
        perror("Gagal alokasi LinkedListNode");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void appendToList(LinkedListNode** head, void* data) {
    LinkedListNode* newNode = createLinkedListNode(data);
    if (*head == NULL) {
        *head = newNode;
    } else {
        LinkedListNode* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void removeFromList(LinkedListNode** head, void* data_to_remove) {
    if (!*head || !data_to_remove) return;
    
    LinkedListNode* current = *head;
    LinkedListNode* prev = NULL;
    
    if (current->data == data_to_remove) {
        *head = current->next;
        free(current);
        return;
    }
    
    while (current != NULL && current->data != data_to_remove) {
        prev = current;
        current = current->next;
    }
    
    if (current != NULL) {
        prev->next = current->next;
        free(current);
    }
} 