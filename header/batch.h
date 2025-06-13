#ifndef BATCH_H
#define BATCH_H

#include "utils.h"

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

// External declarations
extern Queue* batch_task_queue;

// Function prototypes
void processBatchDeleteTasks(Project* project);
void processBatchFile(const char* filename);

#endif // BATCH_H 