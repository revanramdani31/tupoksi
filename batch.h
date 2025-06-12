#ifndef BATCH_H
#define BATCH_H

#include "utils.h"

// BatchDeleteItem structure
struct BatchDeleteItem {
    char taskId[MAX_ID_LEN];
};

// External declarations
extern Queue* batch_task_queue;

// Function prototypes
void processBatchDeleteTasks(Project* project);

#endif // BATCH_H 