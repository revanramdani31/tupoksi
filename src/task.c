// task.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "stack.h"
#include "project.h"
#include "utils.h"
#include "undo.h"
#include "queue.h"

extern Stack* undo_stack;

const char* taskStatusToString[] = {
    "Baru",
    "Dalam Proses",
    "Selesai",
    "Dibatalkan"
};

// Global stack for task history
static Stack* taskHistoryStack = NULL;
Queue* taskCompletionQueue = NULL;

Task* createTaskAndRecordUndo(const char* name, const char* desc, struct Project* project,
                             Task* parentTask, TaskStatus status, const char* dueDate) {
    if (!project || !name || !desc) {
        printf("Error: Data tugas tidak lengkap.\n");
        return NULL;
    }

    // Additional validation for parent task
    if (parentTask) {
        if (strcmp(parentTask->projectId, project->projectId) != 0) {
            printf("Error: Tugas induk harus berada dalam proyek yang sama.\n");
            return NULL;
        }
        if (parentTask->status == TASK_STATUS_DIBATALKAN) {
            printf("Error: Tidak dapat menambahkan sub-tugas ke tugas yang sudah dibatalkan.\n");
            return NULL;
        }
    }

    char new_id[MAX_ID_LEN];
    strncpy(new_id, generateTaskId("TSK"), MAX_ID_LEN-1);
    new_id[MAX_ID_LEN-1] = '\0';

    const char* parent_id = (parentTask != NULL) ? parentTask->taskId : "";
    Task* new_task = createTaskInternal(new_id, name, desc, project->projectId,
                                      parent_id, status, dueDate);

    if (!new_task) {
        printf("Error: Gagal membuat tugas baru.\n");
        return NULL;
    }

    // Record undo action
    UndoAction* action = (UndoAction*)malloc(sizeof(UndoAction));
    if (action) {
        action->type = UNDO_TASK_CREATION;
        strncpy(action->taskId, new_task->taskId, MAX_ID_LEN - 1);
        strncpy(action->projectId, project->projectId, MAX_ID_LEN - 1);
        pushUndoAction(undo_stack, action);
    }

    // Record task creation in history
    recordTaskStatusChange(new_task->taskId, TASK_STATUS_BARU, status, "SYSTEM");
    
    // Create descriptive change message
    char changeMsg[MAX_DESC_LEN];
    if (parentTask) {
        snprintf(changeMsg, MAX_DESC_LEN, "Tugas baru '%s' dibuat sebagai sub-tugas dari '%s'",
                new_task->taskName, parentTask->taskName);
    } else {
        snprintf(changeMsg, MAX_DESC_LEN, "Tugas baru '%s' dibuat sebagai tugas utama",
                new_task->taskName);
    }
    recordChange(changeMsg, "SYSTEM", "TASK_CREATION");

    // Add to project's task tree
    if (parentTask) {
        addChildTask(parentTask, new_task);
        printf("Tugas berhasil ditambahkan sebagai sub-tugas dari '%s'.\n", parentTask->taskName);
    } else {
        new_task->parent = NULL;
        if (!project->rootTasks) {
            project->rootTasks = new_task;
        } else {
            Task* sibling = project->rootTasks;
            while (sibling->nextSibling) {
                sibling = sibling->nextSibling;
            }
            sibling->nextSibling = new_task;
        }
        printf("Tugas berhasil ditambahkan sebagai tugas utama.\n");
    }
        if (new_task && new_task->firstChild == NULL) {
        // Jika ya, masukkan pointer tugas ini ke dalam antrian penyelesaian.
        enqueue(taskCompletionQueue, new_task);
        printf("Info: Tugas '%s' adalah leaf node dan ditambahkan ke antrian penyelesaian.\n", new_task->taskName);
    }

    return new_task;
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
    if (choice > 0 && choice <= TASK_STATUS_COUNT) return (TaskStatus)(choice - 1);
    printf("Pilihan tidak valid, status diatur ke BARU.\n");
    return TASK_STATUS_BARU;
}

Task* createTaskInternal(const char* id, const char* name, const char* desc,
                        const char* projectId, const char* parentTaskId,
                        TaskStatus status, const char* dueDate) {
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

    strncpy(newTask->parentTaskId, parentTaskId, MAX_ID_LEN - 1);
    newTask->parentTaskId[MAX_ID_LEN - 1] = '\0';

    newTask->status = status;

    if (dueDate && strlen(dueDate) > 0) {
        strncpy(newTask->dueDate, dueDate, DATE_LEN - 1);
    } else {
        newTask->dueDate[0] = '\0';
    }
    newTask->dueDate[DATE_LEN - 1] = '\0';

    newTask->parent = NULL;
    newTask->firstChild = NULL;
    newTask->nextSibling = NULL;

    return newTask;
}

void addChildTask(Task* parentTask, Task* childTask) {
    if (!parentTask || !childTask) return;
    childTask->parent = parentTask;

    if (!parentTask->firstChild) {
        parentTask->firstChild = childTask;
    } else {
        Task* sibling = parentTask->firstChild;
        while (sibling->nextSibling) {
            sibling = sibling->nextSibling;
        }
        sibling->nextSibling = childTask;
    }
}

Task* findTaskById(Task* root, const char* taskId) {
    if (!root || !taskId) return NULL;

    if (strcmp(root->taskId, taskId) == 0) {
        return root;
    }

    Task* found = findTaskById(root->firstChild, taskId);
    if (found) return found;

    return findTaskById(root->nextSibling, taskId);
}

void displayWBSHeader() {
    printf("\n+================ WORK BREAKDOWN STRUCTURE ================+\n");
}

void displayWBSFooter() {
    printf("+======================================================+\n");
}

void displayWBSTree(Task* task, int level, int isLastChild) {
    if (!task) return;

    // Print indentation and tree branches with decorative ASCII art
    for (int i = 0; i < level - 1; i++) {
        printf("%s", isLastChild ? "    " : "|   ");
    }
    
    if (level > 0) {
        printf("%s", isLastChild ? "`-- " : "+-- ");
    }

    // Print task status indicator with box drawing
    char* statusSymbol;
    switch (task->status) {
        case TASK_STATUS_BARU:
            statusSymbol = "[ ]";  // Empty box for new tasks
            break;
        case TASK_STATUS_DALAM_PROSES:
            statusSymbol = "[~]";  // Tilde for in-progress
            break;
        case TASK_STATUS_SELESAI:
            statusSymbol = "[*]";  // Star for completed
            break;
        case TASK_STATUS_DIBATALKAN:
            statusSymbol = "[X]";  // X for cancelled
            break;
        default:
            statusSymbol = "[?]";
            break;
    }

    // Print task information with decorative elements
    printf("%s %s", statusSymbol, task->taskName);
    printf(" {%s}", task->taskId);
    if (strlen(task->dueDate) > 0) {
        printf(" [Due: %s]", task->dueDate);
    }
    printf("\n");

    // Print task description with decorative indentation if exists
    if (strlen(task->description) > 0) {
        for (int i = 0; i < level; i++) {
            printf("%s", isLastChild ? "    " : "|   ");
        }
        printf("|   > %s\n", task->description);
    }

    // Add decorative connecting lines between parent and children
    if (task->firstChild) {
        for (int i = 0; i < level; i++) {
            printf("%s", isLastChild ? "    " : "|   ");
        }
        printf("|   |\n");  // Vertical connector
    }

    // Process children
    Task* child = task->firstChild;
    while (child) {
        Task* nextChild = child->nextSibling;
        
        // Add a decorative connecting line before each child except the last one
        if (child != task->firstChild) {
            for (int i = 0; i < level; i++) {
                printf("%s", isLastChild ? "    " : "|   ");
            }
            printf("|   |\n");  // Vertical connector between siblings
        }
        
        displayWBSTree(child, level + 1, nextChild == NULL);
        child = nextChild;
    }

    // Add decorative spacing after a group of siblings
    if (level == 0 && !task->nextSibling) {
        printf("\n");
    }
}

void deepFreeTask(Task* task) {
    if (!task) return;

    // First free all children
    Task* child = task->firstChild;
    while (child) {
        Task* next = child->nextSibling;
        deepFreeTask(child);
        child = next;
    }

    // Then free this task
    free(task);
}

void displayTasksByStatus(Task* root, TaskStatus status, int level, int* count) {
    if (!root) return;

    if (root->status == status) {
        for (int i = 0; i < level; ++i) printf("  ");
        printf("|- [%s] %s (Due: %s)\n",
               root->taskId, root->taskName,
               strlen(root->dueDate) > 0 ? root->dueDate : "N/A");
        (*count)++;
    }

    Task* child = root->firstChild;
    while (child) {
        displayTasksByStatus(child, status, level + 1, count);
        child = child->nextSibling;
    }
}

void displayTasksBySearchTerm(Task* root, const char* searchTerm, int level, int* count) {
    if (!root || !searchTerm) return;

    if (strstr(root->taskName, searchTerm) != NULL ||
        strstr(root->description, searchTerm) != NULL) {
        for (int i = 0; i < level; ++i) printf("  ");
        printf("|- [%s] %s (Status: %s, Due: %s)\n",
               root->taskId, root->taskName,
               taskStatusToString[root->status],
               strlen(root->dueDate) > 0 ? root->dueDate : "N/A");
        (*count)++;
    }

    Task* child = root->firstChild;
    while (child) {
        displayTasksBySearchTerm(child, searchTerm, level + 1, count);
        child = child->nextSibling;
    }
}

Task* findTaskInProjectById(Project* project, const char* taskId) {
    if (!project || !taskId) return NULL;
    return findTaskById(project->rootTasks, taskId);
}


void deleteTask(Project* project, const char* taskId, int isUndo) {
    if (!project || !taskId) return;

    Task* task = findTaskInProjectById(project, taskId);
    if (!task) {
        printf("Tugas ID '%s' tidak ditemukan.\n", taskId);
        return;
    }

    Task* parent = task->parent;

    if (!isUndo) {
        char description[MAX_DESC_LEN];
        snprintf(description, MAX_DESC_LEN, "Tugas '%s' (ID: %s) dihapus", task->taskName, task->taskId);
        recordChange(description, "SYSTEM", "TASK_DELETION");
    }

    if (task->parent) {
        Task* prev = NULL;
        Task* current = task->parent->firstChild;
        while (current && current != task) {
            prev = current;
            current = current->nextSibling;
        }
        if (current) {
            if (prev) {
                prev->nextSibling = current->nextSibling;
            } else {
                task->parent->firstChild = current->nextSibling;
            }
        }
    } else {
        Task* prev = NULL;
        Task* current = project->rootTasks;
        while (current && current != task) {
            prev = current;
            current = current->nextSibling;
        }
        if (current) {
            if (prev) {
                prev->nextSibling = current->nextSibling;
            } else {
                project->rootTasks = current->nextSibling;
            }
        }
    }

    deepFreeTask(task);

    if (parent && parent->firstChild == NULL) {
        if (taskCompletionQueue) {
            enqueue(taskCompletionQueue, parent);
            printf("Info: Tugas '%s' menjadi leaf node dan ditambahkan ke antrian penyelesaian.\n", parent->taskName);
        }
    }

    if (!isUndo) {
        printf("Tugas ID '%s' berhasil dihapus.\n", taskId);
    }
}

void editTask(Task* task) {
    if (!task) return;

    char buffer[MAX_NAME_LEN];
    TaskStatus oldStatus = task->status;

    printf("\nEdit Tugas ID: %s\n", task->taskId);
    printf("Nama saat ini: %s\n", task->taskName);
    printf("Masukkan nama baru (kosongkan jika tidak ingin ubah): ");
    fgets(buffer, MAX_NAME_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) {
        strncpy(task->taskName, buffer, MAX_NAME_LEN - 1);
        task->taskName[MAX_NAME_LEN - 1] = '\0';
    }

    printf("Deskripsi saat ini: %s\n", task->description);
    printf("Masukkan deskripsi baru (kosongkan jika tidak ingin ubah): ");
    fgets(buffer, MAX_DESC_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) {
        strncpy(task->description, buffer, MAX_DESC_LEN - 1);
        task->description[MAX_DESC_LEN - 1] = '\0';
    }

    printf("Status saat ini: %s\n", taskStatusToString[task->status]);
    TaskStatus newStatus = getTaskStatusFromInput();
    task->status = newStatus;

    printf("Tenggat saat ini: %s\n", strlen(task->dueDate) > 0 ? task->dueDate : "N/A");
    printf("Masukkan tenggat baru (YYYY-MM-DD, kosongkan jika tidak ingin ubah): ");
    fgets(buffer, DATE_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) {
        strncpy(task->dueDate, buffer, DATE_LEN - 1);
        task->dueDate[DATE_LEN - 1] = '\0';
    }

    if (oldStatus != newStatus) {
        recordTaskStatusChange(task->taskId, oldStatus, newStatus, "SYSTEM");
        recordChange("Status tugas diubah", "SYSTEM", "TASK_STATUS_CHANGE");
    }

    int isNowActive = (newStatus == TASK_STATUS_BARU || newStatus == TASK_STATUS_DALAM_PROSES);
    int wasPreviouslyFinished = (oldStatus == TASK_STATUS_SELESAI || oldStatus == TASK_STATUS_DIBATALKAN);

    if (task->firstChild == NULL && isNowActive && wasPreviouslyFinished) {
        if (taskCompletionQueue) {
            enqueue(taskCompletionQueue, task);
            printf("Info: Tugas '%s' dikembalikan ke antrian penyelesaian karena statusnya diubah menjadi aktif.\n", task->taskName);
        }
    }

    printf("Tugas berhasil diperbarui.\n");
}

void addRootTaskToProject(Project* project, Task* task) {
    if (!project || !task) return;

    task->parent = NULL;
    if (!project->rootTasks) {
        project->rootTasks = task;
    } else {
        Task* current = project->rootTasks;
        while (current->nextSibling) {
            current = current->nextSibling;
        }
        current->nextSibling = task;
    }
}

void recordTaskStatusChange(const char* taskId, TaskStatus oldStatus, 
                          TaskStatus newStatus, const char* userId) {
    if (!taskId || !userId) return;

    if (!taskHistoryStack) {
        taskHistoryStack = createStack();
        if (!taskHistoryStack) {
            printf("ERROR: Gagal membuat task history stack.\n");
            return;
        }
    }

    TaskHistory* history = (TaskHistory*)malloc(sizeof(TaskHistory));
    if (!history) {
        perror("Gagal alokasi TaskHistory");
        return;
    }

    strncpy(history->taskId, taskId, MAX_ID_LEN - 1);
    history->taskId[MAX_ID_LEN - 1] = '\0';

    history->oldStatus = oldStatus;
    history->newStatus = newStatus;

    time_t now = time(NULL);
    strftime(history->timestamp, DATE_LEN, "%Y-%m-%d", localtime(&now));

    strncpy(history->userId, userId, MAX_ID_LEN - 1);
    history->userId[MAX_ID_LEN - 1] = '\0';

    pushUndoAction(taskHistoryStack, (UndoAction*)history);
}

void displayTaskHistory(const char* taskId) {
    if (!taskId || !taskHistoryStack) return;

    printf("\nRiwayat Status Tugas [%s]:\n", taskId);
    printf("----------------------------------------\n");
    printf("Tanggal    | Status Lama -> Status Baru | User\n");
    printf("----------------------------------------\n");

    Stack* tempStack = createStack();
    if (!tempStack) return;

    // Pop all items to temp stack
    while (!isEmpty(taskHistoryStack)) {
        TaskHistory* history = (TaskHistory*)popUndoAction(taskHistoryStack);
        if (history && strcmp(history->taskId, taskId) == 0) {
            printf("%s | %s -> %s | %s\n",
                   history->timestamp,
                   taskStatusToString[history->oldStatus],
                   taskStatusToString[history->newStatus],
                   history->userId);
        }
        pushUndoAction(tempStack, (UndoAction*)history);
    }

    // Restore items back to original stack
    while (!isEmpty(tempStack)) {
        TaskHistory* history = (TaskHistory*)popUndoAction(tempStack);
        pushUndoAction(taskHistoryStack, (UndoAction*)history);
    }

    freeStack(tempStack);
    printf("----------------------------------------\n");
}

void analyzeTaskStatusChanges(const char* taskId) {
    if (!taskId || !taskHistoryStack) return;

    int statusCounts[TASK_STATUS_COUNT] = {0};
    int totalChanges = 0;

    Stack* tempStack = createStack();
    if (!tempStack) return;

    // Count status changes
    while (!isEmpty(taskHistoryStack)) {
        TaskHistory* history = (TaskHistory*)popUndoAction(taskHistoryStack);
        if (history && strcmp(history->taskId, taskId) == 0) {
            statusCounts[history->newStatus]++;
            totalChanges++;
        }
        pushUndoAction(tempStack, (UndoAction*)history);
    }

    // Restore items back to original stack
    while (!isEmpty(tempStack)) {
        TaskHistory* history = (TaskHistory*)popUndoAction(tempStack);
        pushUndoAction(taskHistoryStack, (UndoAction*)history);
    }

    freeStack(tempStack);

    // Display analysis
    printf("\nAnalisis Status Tugas [%s]:\n", taskId);
    printf("----------------------------------------\n");
    printf("Total perubahan status: %d\n", totalChanges);
    printf("\nDistribusi Status:\n");
    for (int i = 0; i < TASK_STATUS_COUNT; i++) {
        float percentage = totalChanges > 0 ? (float)statusCounts[i] / totalChanges * 100 : 0;
        printf("%s: %d kali (%.1f%%)\n", 
               taskStatusToString[i], statusCounts[i], percentage);
    }
    printf("----------------------------------------\n");
}

void countTasksAndStatus(Task* task, int* totalTasks, int* statusCounts) {
    if (!task) return;

    // Count this task
    (*totalTasks)++;
    statusCounts[task->status]++;

    // Count children
    Task* child = task->firstChild;
    while (child) {
        countTasksAndStatus(child, totalTasks, statusCounts);
        child = child->nextSibling;
    }
}

void displayCompletionQueue() {
    if (!taskCompletionQueue || isQueueEmpty(taskCompletionQueue)) {
        printf("\nAntrian tugas untuk diselesaikan kosong.\n");
        return;
    }

    printf("\n--- Antrian Tugas untuk Diselesaikan (Urutan FIFO) ---\n");

    Queue* tempQueue = createQueue();
    int count = 1;

    while (!isQueueEmpty(taskCompletionQueue)) {
        Task* task = (Task*)dequeue(taskCompletionQueue);
        if (task) {
            printf("%d. [%s] %s\n", count++, task->taskId, task->taskName);
            enqueue(tempQueue, task);
        }
    }

    while (!isQueueEmpty(tempQueue)) {
        enqueue(taskCompletionQueue, dequeue(tempQueue));
    }

    free(tempQueue);
    printf("-------------------------------------------------------\n");
}

// task.c

// task.c

// GANTI DENGAN VERSI FINAL YANG PALING LENGKAP INI
void processNextTaskInQueue() {
    if (!taskCompletionQueue || isQueueEmpty(taskCompletionQueue)) {
        printf("\nTidak ada tugas dalam antrian untuk diselesaikan.\n");
        return;
    }

    // "Intip" tugas di depan antrian
    Task* taskToProcess = (Task*)peekQueue(taskCompletionQueue);
    if (!taskToProcess) {
        dequeue(taskCompletionQueue); // Hapus item kosong/rusak jika ada
        return;
    }

    // --- VERIFIKASI #1: Apakah tugas masih leaf node? ---
    if (taskToProcess->firstChild != NULL) {
        printf("\nInfo: Melewati tugas [%s] %s karena sudah menjadi tugas induk (bukan leaf node lagi).\n",
               taskToProcess->taskId, taskToProcess->taskName);
        dequeue(taskCompletionQueue); // Keluarkan dari antrian karena tidak relevan
        processNextTaskInQueue();     // Coba proses item berikutnya
        return;
    }

    // --- VERIFIKASI #2 (BARU): Apakah status tugas sudah selesai/dibatalkan? ---
    if (taskToProcess->status == TASK_STATUS_SELESAI || taskToProcess->status == TASK_STATUS_DIBATALKAN) {
        printf("\nInfo: Melewati tugas [%s] %s karena statusnya sudah '%s'.\n",
               taskToProcess->taskId, taskToProcess->taskName, taskStatusToString[taskToProcess->status]);
        dequeue(taskCompletionQueue); // Keluarkan dari antrian karena tidak relevan
        processNextTaskInQueue();     // Coba proses item berikutnya
        return;
    }

    // Jika lolos semua verifikasi, lanjutkan dengan konfirmasi pengguna
    char input[10];
    printf("\nAnda akan menyelesaikan tugas: [%s] %s\n", taskToProcess->taskId, taskToProcess->taskName);
    printf("Lanjutkan? (y/n): ");
    fgets(input, sizeof(input), stdin);

    if (input[0] == 'y' || input[0] == 'Y') {
        dequeue(taskCompletionQueue); // Proses dan keluarkan dari antrian

        TaskStatus oldStatus = taskToProcess->status;
        taskToProcess->status = TASK_STATUS_SELESAI;
        recordTaskStatusChange(taskToProcess->taskId, oldStatus, TASK_STATUS_SELESAI, "SYSTEM");
        
        char changeMsg[MAX_DESC_LEN];
        snprintf(changeMsg, MAX_DESC_LEN, "Tugas '%s' diselesaikan dari antrian.", taskToProcess->taskName);
        recordChange(changeMsg, "SYSTEM", "TASK_COMPLETION");

        printf("Berhasil! Tugas '%s' telah ditandai sebagai Selesai.\n", taskToProcess->taskName);
    } else {
        printf("Penyelesaian tugas dibatalkan. Tugas tetap berada di antrian.\n");
    }
}
// Fungsi untuk mengisi ulang antrian dari tree setelah load data
void repopulateCompletionQueue(Task* task) {
    if (task == NULL) {
        return; // Basis untuk menghentikan rekursi
    }

    // Langkah 1: Proses node yang sedang dikunjungi saat ini.
    // Hanya jika node ini TIDAK punya anak, maka ia adalah leaf node.
    if (task->firstChild == NULL) {
        if (taskCompletionQueue) {
            enqueue(taskCompletionQueue, task);
        }
    }

    // Langkah 2: Lakukan traversal ke semua anak dari node ini.
    repopulateCompletionQueue(task->firstChild);

    // Langkah 3: Lanjutkan traversal ke semua saudara dari node ini.
    repopulateCompletionQueue(task->nextSibling);
}
