#ifndef STACK_H
#define STACK_H

#include "utils.h"
#include "undo.h"

typedef struct StackNode {
    void* data;
    struct StackNode* next;
} StackNode;

typedef struct Stack {
    StackNode* top;
    int size;
} Stack;

Stack* createStack();
void push(Stack* s, void* data);
void* pop(Stack* s);
void* peek(Stack* s);
int isEmpty(Stack* s);
void freeStack(Stack* s);

void pushUndoAction(Stack* s, UndoAction* action);
UndoAction* popUndoAction(Stack* s);
void freeStackAndActions(Stack* s);

#endif 
