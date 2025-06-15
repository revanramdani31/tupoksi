#ifndef UNDO_H
#define UNDO_H

#include "utils.h"

#define MAX_ID_LEN 20

typedef enum {
    UNDO_TASK_CREATION,
    UNDO_TASK_DELETION,
    UNDO_PROJECT_CREATION,
    UNDO_PROJECT_DELETION
} UndoActionType;

typedef struct {
    UndoActionType type;
    char taskId[MAX_ID_LEN];
    char projectId[MAX_ID_LEN];
} UndoAction;

struct Stack;
typedef struct Stack Stack;

extern Stack* undo_stack;

void pushUndoAction(Stack* s, UndoAction* action);
UndoAction* popUndoAction(Stack* s);
void processUndoLastTaskCreation();

#endif
