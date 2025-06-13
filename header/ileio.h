#ifndef ILEIO_H
#define ILEIO_H

#include "utils.h"
#include "project.h"
#include "task.h"

// Function prototypes
void saveTasksOfProject(FILE* file_ptr, Task* task);
void saveDataToFile(const char* filename);
void buildTaskHierarchyForProject(Project* project, Task** all_tasks, int task_count);
void loadDataFromFile(const char* filename);

#endif // ILEIO_H 