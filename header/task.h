
#ifndef TASK_H
#define TASK_H

#include <time.h>

typedef struct Project Project;

#define MAX_NAME_LEN 100
#define MAX_DESC_LEN 500
#define MAX_ID_LEN 20
#define DATE_LEN 11

typedef enum {
    TASK_STATUS_BARU,
    TASK_STATUS_DALAM_PROSES,
    TASK_STATUS_SELESAI,
    TASK_STATUS_DIBATALKAN,
    TASK_STATUS_COUNT
} TaskStatus;

typedef struct Task {
    char taskId[MAX_ID_LEN];
    char taskName[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    char projectId[MAX_ID_LEN];
    char parentTaskId[MAX_ID_LEN];
    TaskStatus status;
    char dueDate[DATE_LEN];
    struct Task* parent;
    struct Task* firstChild;
    struct Task* nextSibling;
} Task;

typedef struct {
    char taskId[MAX_ID_LEN];
    TaskStatus oldStatus;
    TaskStatus newStatus;
    char timestamp[DATE_LEN];
    char userId[MAX_ID_LEN];
} TaskHistory;

Task* createTaskInternal(const char* id, const char* name, const char* desc,
                        const char* projectId, const char* parentTaskId,
                        TaskStatus status, const char* dueDate);
Task* createTaskAndRecordUndo(const char* name, const char* desc, Project* project,
                             Task* parentTask, TaskStatus status, const char* dueDate);
void addChildTask(Task* parent, Task* child);
Task* findTaskById(Task* root, const char* taskId);
Task* findTaskInProjectById(Project* project, const char* taskId);
void editTask(Task* task);
void deleteTask(Project* project, const char* taskId, int isUndo);
void deepFreeTask(Task* task);

void displayWBSTree(Task* task, int level, int isLastChild);
void displayTaskDetails(Task* task);
void displayTasksByStatus(Task* root, TaskStatus status, int level, int* count);
void displayTasksBySearchTerm(Task* root, const char* searchTerm, int level, int* count);
void countTasksAndStatus(Task* task, int* totalTasks, int* statusCounts);

TaskStatus getTaskStatusFromInput();
const char* getTaskStatusString(TaskStatus status);

void recordTaskStatusChange(const char* taskId, TaskStatus oldStatus, 
                          TaskStatus newStatus, const char* userId);
void displayTaskHistory(const char* taskId);
void analyzeTaskStatusChanges(const char* taskId);
// task.c

void displayCompletionQueue();
void processNextTaskInQueue();
void repopulateCompletionQueue(Task* task);
extern const char* taskStatusToString[];

#endif
