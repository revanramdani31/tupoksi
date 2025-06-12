#ifndef UNDO_H
#define UNDO_H

#include "utils.h"
#include "stack.h"

// UndoActionType enum
typedef enum {
    UNDO_ACTION_NONE,
    UNDO_TASK_CREATION
} UndoActionType;

// UndoAction structure
struct UndoAction {
    UndoActionType type;
    char taskId[MAX_ID_LEN];
    char projectId[MAX_ID_LEN];
};

// Undo action types
#define UNDO_TASK_CREATION 1

// External declarations
extern Stack* undo_stack;

// Function prototypes
void pushUndoAction(Stack* s, UndoAction* action);
void processUndoLastTaskCreation();

#endif // UNDO_H 