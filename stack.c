#include "stack.h"
#include "linkedlist.h"

Stack* createStack(int limit) {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    if (!s) {
        perror("Gagal alokasi Stack");
        exit(EXIT_FAILURE);
    }
    s->top = NULL;
    s->count = 0;
    s->limit = limit;
    return s;
}

int isStackEmpty(Stack* s) {
    return s == NULL || s->top == NULL;
}

int isStackFull(Stack* s) {
    return s != NULL && s->limit > 0 && s->count >= s->limit;
}

void pushUndoAction(Stack* s, UndoAction* action) {
    if (!s || !action) return;
    
    if (isStackFull(s)) {
        printf("INFO: Undo stack penuh. Aksi terlama tidak dihapus (implementasi sederhana).\n");
        if (s->limit > 0) {
            free(action);
            return;
        }
    }
    
    LinkedListNode* newNode = createLinkedListNode(action);
    if (!newNode) {
        free(action);
        perror("Gagal alokasi Node untuk Stack Undo");
        return;
    }
    
    newNode->next = s->top;
    s->top = newNode;
    s->count++;
}

UndoAction* popUndoAction(Stack* s) {
    if (isStackEmpty(s)) return NULL;
    
    LinkedListNode* temp = s->top;
    s->top = s->top->next;
    UndoAction* action = (UndoAction*)temp->data;
    free(temp);
    s->count--;
    return action;
}

void freeStack(Stack* s) {
    if (!s) return;
    
    while (!isStackEmpty(s)) {
        UndoAction* action = popUndoAction(s);
        if (action) free(action);
    }
    free(s);
}

void freeStackAndActions(Stack* s) {
    if (!s) return;
    
    while (!isStackEmpty(s)) {
        UndoAction* action = popUndoAction(s);
        if (action) free(action);
    }
    free(s);
} 