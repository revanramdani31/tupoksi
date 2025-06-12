#ifndef ILEIO_H
#define ILEIO_H

#include "utils.h"

// Function prototypes
void saveTasksOfProject(FILE* file_ptr, LinkedListNode* task_head_node);
void saveDataToFile(const char* filename);
void buildTaskHierarchyForProject(Project* project, LinkedListNode** all_tasks_for_project_head);
void loadDataFromFile(const char* filename);

#endif // ILEIO_H 