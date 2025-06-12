#include "task.h"
#include "project.h"
#include "linkedlist.h"
#include "undo.h"

// External declarations
extern const char* taskStatusToString[];
extern const char* taskPriorityToString[];
extern LinkedListNode* project_list_head;

Task* createTaskInternal(const char* id, const char* name, const char* desc,
                        const char* projectId, const char* parentIdStr,
                        TaskStatus status, TaskPriority priority, const char* dueDate) {
    Task* newTask = (Task*)malloc(sizeof(Task));
    if (!newTask) {
        perror("Gagal alokasi Task");
        return NULL;
    }

    strncpy(newTask->taskId, id, MAX_ID_LEN - 1);
    newTask->taskId[MAX_ID_LEN - 1] = '\0';

    strncpy(newTask->taskName, name, MAX_NAME_LEN - 1);
    newTask->taskName[MAX_NAME_LEN - 1] = '\0';
    
    strncpy(newTask->description, desc, MAX_DESC_LEN - 1);
    newTask->description[MAX_DESC_LEN - 1] = '\0';
    
    strncpy(newTask->projectId, projectId, MAX_ID_LEN - 1);
    newTask->projectId[MAX_ID_LEN - 1] = '\0';
    
    if (parentIdStr && strcmp(parentIdStr, "NULL") != 0 && strlen(parentIdStr) > 0) {
        strncpy(newTask->parentTaskId, parentIdStr, MAX_ID_LEN - 1);
        newTask->parentTaskId[MAX_ID_LEN - 1] = '\0';
    } else {
        newTask->parentTaskId[0] = '\0';
    }
    
    newTask->status = status;
    newTask->priority = priority;
    
    if (dueDate && strlen(dueDate) > 0) {
        strncpy(newTask->dueDate, dueDate, DATE_LEN - 1);
    } else {
        newTask->dueDate[0] = '\0';
    }
    newTask->dueDate[DATE_LEN - 1] = '\0';
    
    newTask->parent = NULL;
    newTask->children_head = NULL;
    return newTask;
}

Task* createTaskAndRecordUndo(const char* name, const char* desc, Project* project,
                             Task* parentTaskOpt, TaskStatus status, TaskPriority priority,
                             const char* dueDate) {
    if (!project) return NULL;
    
    char new_id[MAX_ID_LEN] = {0};
    strncpy(new_id, generateUniqueId("TSK"), MAX_ID_LEN-1);
    new_id[MAX_ID_LEN-1] = '\0';
    
    const char* parentIdStr = (parentTaskOpt) ? parentTaskOpt->taskId : "NULL";
    Task* newTask = createTaskInternal(new_id, name, desc, project->projectId,
                                     parentIdStr, status, priority, dueDate);
    
    if (newTask) {
        UndoAction* action = (UndoAction*)malloc(sizeof(UndoAction));
        if (action) {
            action->type = UNDO_TASK_CREATION;
            strncpy(action->taskId, newTask->taskId, MAX_ID_LEN - 1);
            action->taskId[MAX_ID_LEN - 1] = '\0';
            strncpy(action->projectId, project->projectId, MAX_ID_LEN - 1);
            action->projectId[MAX_ID_LEN - 1] = '\0';
            pushUndoAction(undo_stack, action);
        } else {
            printf("ERROR: Gagal buat action undo.\n");
            free(newTask);
            return NULL;
        }
    }
    return newTask;
}

void addRootTaskToProject(Project* project, Task* task) {
    if (!project || !task) return;
    task->parent = NULL;
    task->parentTaskId[0] = '\0';
    appendToList(&(project->tasks_head), task);
    printf("Tugas utama '%s' ke proyek '%s'.\n", task->taskName, project->projectName);
}

void addChildTask(Task* parentTask, Task* childTask) {
    if (!parentTask || !childTask) return;
    childTask->parent = parentTask;
    strcpy(childTask->parentTaskId, parentTask->taskId);
    appendToList(&(parentTask->children_head), childTask);
    printf("Sub-tugas '%s' ditambahkan ke '%s'.\n", childTask->taskName, parentTask->taskName);
}

Task* findTaskInProjectRecursive(LinkedListNode* task_node_head, const char* taskId) {
    LinkedListNode* current_node = task_node_head;
    while (current_node != NULL) {
        Task* current_task = (Task*)current_node->data;
        if (strcmp(current_task->taskId, taskId) == 0) return current_task;
        
        Task* found_in_child = findTaskInProjectRecursive(current_task->children_head, taskId);
        if (found_in_child) return found_in_child;
        
        current_node = current_node->next;
    }
    return NULL;
}

Task* findTaskInProjectById(Project* project, const char* taskId) {
    if (!project || !taskId) return NULL;
    return findTaskInProjectRecursive(project->tasks_head, taskId);
}

void deepFreeTask(Task* task) {
    if (!task) return;
    
    LinkedListNode* child_node = task->children_head;
    LinkedListNode* next_child_node;
    
    while (child_node != NULL) {
        next_child_node = child_node->next;
        deepFreeTask((Task*)child_node->data);
        free(child_node);
        child_node = next_child_node;
    }
    
    free(task);
}

void deleteTask(Project* project, const char* taskId, int an_undo_action) {
    if (!project || !taskId) {
        printf("ERROR: Proyek/ID Tugas tidak valid untuk dihapus.\n");
        return;
    }
    
    Task* task_to_delete = findTaskInProjectById(project, taskId);
    if (!task_to_delete) {
        printf("ERROR: Tugas ID '%s' tidak ditemukan di proyek '%s'.\n", taskId, project->projectName);
        return;
    }
    
    if (task_to_delete->parent) {
        removeFromList(&(task_to_delete->parent->children_head), task_to_delete);
    } else {
        removeFromList(&(project->tasks_head), task_to_delete);
    }
    
    char taskNameCopy[MAX_NAME_LEN];
    strncpy(taskNameCopy, task_to_delete->taskName, MAX_NAME_LEN - 1);
    taskNameCopy[MAX_NAME_LEN - 1] = '\0';
    
    deepFreeTask(task_to_delete);
    
    if (!an_undo_action) {
        printf("Tugas '%s' (ID: %s) dan sub-tugasnya dihapus.\n", taskNameCopy, taskId);
    }
}

TaskStatus getTaskStatusFromInput() {
    int choice = 0;
    printf("Pilih Status Tugas:\n");
    for (int i = 0; i < TASK_STATUS_COUNT; i++) {
        printf("%d. %s\n", i + 1, taskStatusToString[i]);
    }
    printf("Pilihan Status (1-%d): ", TASK_STATUS_COUNT);
    if (scanf("%d", &choice) != 1) choice = 0;
    while(getchar() != '\n');
    
    if (choice > 0 && choice <= TASK_STATUS_COUNT) {
        return (TaskStatus)(choice - 1);
    }
    printf("Pilihan tidak valid, status diatur ke BARU.\n");
    return TASK_STATUS_BARU;
}

TaskPriority getTaskPriorityFromInput() {
    int choice = 0;
    printf("Pilih Prioritas Tugas:\n");
    for (int i = 0; i < TASK_PRIORITY_COUNT; i++) {
        printf("%d. %s\n", i + 1, taskPriorityToString[i]);
    }
    printf("Pilihan Prioritas (1-%d): ", TASK_PRIORITY_COUNT);
    if (scanf("%d", &choice) != 1) choice = 0;
    while(getchar() != '\n');
    
    if (choice > 0 && choice <= TASK_PRIORITY_COUNT) {
        return (TaskPriority)(choice - 1);
    }
    printf("Pilihan tidak valid, prioritas diatur ke SEDANG.\n");
    return TASK_PRIORITY_SEDANG;
}

void editTask(Task* task) {
    if (!task) {
        printf("ERROR: Tugas tidak valid untuk diedit.\n");
        return;
    }

    printf("\n=== EDIT TUGAS ===\n");
    printf("Tugas saat ini: %s\n", task->taskName);
    printf("1. Edit Nama\n");
    printf("2. Edit Deskripsi\n");
    printf("3. Edit Status\n");
    printf("4. Edit Prioritas\n");
    printf("5. Edit Tanggal Tenggat\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");

    int choice;
    scanf("%d", &choice);
    getchar(); // Consume newline

    switch (choice) {
        case 1: {
            char newName[MAX_NAME_LEN];
            printf("Masukkan nama baru: ");
            fgets(newName, MAX_NAME_LEN, stdin);
            newName[strcspn(newName, "\n")] = 0;
            strncpy(task->taskName, newName, MAX_NAME_LEN - 1);
            task->taskName[MAX_NAME_LEN - 1] = '\0';
            printf("Nama tugas berhasil diubah.\n");
            break;
        }
        case 2: {
            char newDesc[MAX_DESC_LEN];
            printf("Masukkan deskripsi baru: ");
            fgets(newDesc, MAX_DESC_LEN, stdin);
            newDesc[strcspn(newDesc, "\n")] = 0;
            strncpy(task->description, newDesc, MAX_DESC_LEN - 1);
            task->description[MAX_DESC_LEN - 1] = '\0';
            printf("Deskripsi tugas berhasil diubah.\n");
            break;
        }
        case 3: {
            printf("Pilih status baru:\n");
            task->status = getTaskStatusFromInput();
            printf("Status tugas berhasil diubah.\n");
            break;
        }
        case 4: {
            printf("Pilih prioritas baru:\n");
            task->priority = getTaskPriorityFromInput();
            printf("Prioritas tugas berhasil diubah.\n");
            break;
        }
        case 5: {
            char newDueDate[DATE_LEN];
            printf("Masukkan tanggal tenggat baru (YYYY-MM-DD, kosongkan jika tidak ada): ");
            fgets(newDueDate, DATE_LEN, stdin);
            newDueDate[strcspn(newDueDate, "\n")] = 0;
            if (strlen(newDueDate) > 0) {
                strncpy(task->dueDate, newDueDate, DATE_LEN - 1);
                task->dueDate[DATE_LEN - 1] = '\0';
            } else {
                task->dueDate[0] = '\0';
            }
            printf("Tanggal tenggat tugas berhasil diubah.\n");
            break;
        }
        case 0:
            return;
        default:
            printf("Pilihan tidak valid.\n");
    }
} 