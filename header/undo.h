#ifndef UNDO_H
#define UNDO_H

#include "utils.h"

#define MAX_ID_LEN 20

// UndoActionType enum
typedef enum {
    UNDO_TASK_CREATION,
    UNDO_TASK_DELETION,
    UNDO_PROJECT_CREATION,
    UNDO_PROJECT_DELETION
} UndoActionType;

// UndoAction structure
typedef struct {
    UndoActionType type;
    char taskId[MAX_ID_LEN];
    char projectId[MAX_ID_LEN];
} UndoAction;

// Forward declarations
struct Stack;
typedef struct Stack Stack;

// External declarations
extern Stack* undo_stack;

// Function prototypes
void pushUndoAction(Stack* s, UndoAction* action);
UndoAction* popUndoAction(Stack* s);
void processUndoLastTaskCreation();

#endif // UNDO_H 