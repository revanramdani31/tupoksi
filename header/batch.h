#ifndef BATCH_H
#define BATCH_H

#include "utils.h"
#include "task.h"

// Forward declarations
struct Queue;
typedef struct Queue Queue;

typedef struct BatchDeleteItem {
    char taskId[MAX_ID_LEN];
} BatchDeleteItem;

typedef struct BatchTask {
    char taskId[MAX_ID_LEN];
    char taskName[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    char projectId[MAX_ID_LEN];
    char parentTaskId[MAX_ID_LEN];
    int status;
    char dueDate[DATE_LEN];
} BatchTask;

typedef enum {
    BATCH_DELETE,
    BATCH_STATUS_CHANGE,
    BATCH_EDIT
} BatchOperationType;

typedef struct {
    char taskId[MAX_ID_LEN];
    TaskStatus newStatus;
} BatchStatusChange;

typedef struct {
    char taskId[MAX_ID_LEN];
    char taskName[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    char dueDate[DATE_LEN];
} BatchEdit;

typedef struct BatchOperation {
    BatchOperationType type;
    union {
        BatchDeleteItem deleteItem;
        BatchStatusChange statusChange;
        BatchEdit editItem;
    } data;
} BatchOperation;

// External declarations
extern Queue* batch_task_queue;

// Function prototypes
void processBatchDeleteTasks(Project* project);
void processBatchStatusChange(Project* project);
void processBatchEdit(Project* project);
void processBatchOperation(Project* project, BatchOperation* operation);

#endif // BATCH_H 