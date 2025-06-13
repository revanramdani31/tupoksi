#include "stack.h"
#include <stdlib.h>

Stack* createStack() {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    if (!s) return NULL;
    s->top = NULL;
    s->size = 0;
    return s;
}

void push(Stack* s, void* data) {
    if (!s) return;
    
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (!newNode) return;
    
    newNode->data = data;
    newNode->next = s->top;
    s->top = newNode;
    s->size++;
}

void* pop(Stack* s) {
    if (!s || !s->top) return NULL;
    
    StackNode* temp = s->top;
    void* data = temp->data;
    s->top = temp->next;
    free(temp);
    s->size--;
    return data;
}

void* peek(Stack* s) {
    if (!s || !s->top) return NULL;
    return s->top->data;
}

int isEmpty(Stack* s) {
    return !s || !s->top;
}

void freeStack(Stack* s) {
    if (!s) return;
    
    while (!isEmpty(s)) {
        void* data = pop(s);
        if (data) free(data);
    }
    free(s);
} 