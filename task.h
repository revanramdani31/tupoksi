#ifndef TASK_H
#define TASK_H

#include "utils.h"

// Enum untuk Atribut Tugas
typedef enum {
    TASK_STATUS_BARU, TASK_STATUS_DIKERJAKAN, TASK_STATUS_SELESAI, TASK_STATUS_TERTUNDA,
    TASK_STATUS_COUNT
} TaskStatus;

typedef enum {
    TASK_PRIORITY_RENDAH, TASK_PRIORITY_SEDANG, TASK_PRIORITY_TINGGI, TASK_PRIORITY_MENDESAK,
    TASK_PRIORITY_COUNT
} TaskPriority;

// Task structure
struct Task {
    char taskId[MAX_ID_LEN];
    char taskName[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    char projectId[MAX_ID_LEN];
    char parentTaskId[MAX_ID_LEN];
    Task* parent;
    LinkedListNode* children_head;
    TaskStatus status;
    TaskPriority priority;
    char dueDate[DATE_LEN];
};

// External declarations
extern const char* taskStatusToString[];
extern const char* taskPriorityToString[];

// Function prototypes
Task* createTaskInternal(const char* id, const char* name, const char* desc,
                        const char* projectId, const char* parentIdStr,
                        TaskStatus status, TaskPriority priority, const char* dueDate);
Task* createTaskAndRecordUndo(const char* name, const char* desc, Project* project,
                             Task* parentTaskOpt, TaskStatus status, TaskPriority priority,
                             const char* dueDate);
void addRootTaskToProject(Project* project, Task* task);
void addChildTask(Task* parentTask, Task* childTask);
Task* findTaskInProjectRecursive(LinkedListNode* task_node_head, const char* taskId);
Task* findTaskInProjectById(Project* project, const char* taskId);
void deepFreeTask(Task* task);
void deleteTask(Project* project, const char* taskId, int an_undo_action);
TaskStatus getTaskStatusFromInput();
TaskPriority getTaskPriorityFromInput();
void editTask(Task* task);

#endif // TASK_H 