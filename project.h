#ifndef PROJECT_H
#define PROJECT_H

#include "utils.h"
#include "task.h"

// Project structure
struct Project {
    char projectId[MAX_ID_LEN];
    char projectName[MAX_NAME_LEN];
    LinkedListNode* tasks_head;
};

// External declarations
extern LinkedListNode* project_list_head;

// Function prototypes
Project* createProjectInternal(const char* id, const char* name);
Project* createProject(const char* name);
Project* findProjectById(const char* projectId);
void editProjectDetails(Project* project);
void deleteProject(const char* projectId);
void listAllProjects();
void displayTaskWBS(Task* task, int level);
void displayProjectWBS(Project* project);
void searchProjectsByName(const char* searchTerm);
void searchTasksInProjectRecursive(LinkedListNode* task_node_head, const char* searchTerm, int* found_count, int level);
void searchTasksInProjectByName(Project* project, const char* searchTerm);
void findAndPrintTasksByStatusRecursive(LinkedListNode* task_head, TaskStatus status_to_find, int* count, int level);
void reportTasksByStatus(Project* project);
void findAndPrintTasksByPriorityRecursive(LinkedListNode* task_head, TaskPriority prio_to_find, int* count, int level);
void reportTasksByPriority(Project* project);

#endif // PROJECT_H 