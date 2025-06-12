#ifndef STACK_H
#define STACK_H

#include "utils.h"

// Stack structure
struct Stack {
    LinkedListNode* top;
    int count;
    int limit;
};

// Function prototypes
Stack* createStack(int limit);
int isStackEmpty(Stack* s);
int isStackFull(Stack* s);
void pushUndoAction(Stack* s, UndoAction* action);
UndoAction* popUndoAction(Stack* s);
void freeStack(Stack* s);
void freeStackAndActions(Stack* s);

#endif // STACK_H 