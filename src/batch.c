#include "batch.h"
#include "project.h"
#include "task.h"
#include "queue.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables
Queue* batch_task_queue = NULL;

void initBatchQueue() {
    if (batch_task_queue) {
        freeQueueAndItems(batch_task_queue);
    }
    batch_task_queue = createQueue();
}

void processBatchOperation(Project* project, BatchOperation* operation) {
    if (!project || !operation) return;

    Task* task = findTaskInProjectById(project, 
        operation->type == BATCH_DELETE ? operation->data.deleteItem.taskId :
        operation->type == BATCH_STATUS_CHANGE ? operation->data.statusChange.taskId :
        operation->type == BATCH_EDIT ? operation->data.editItem.taskId : "");

    if (!task) {
        printf("Tugas tidak ditemukan.\n");
        return;
    }

    // Add confirmation based on operation type
    char confirmMsg[MAX_DESC_LEN];
    switch (operation->type) {
        case BATCH_DELETE:
            snprintf(confirmMsg, MAX_DESC_LEN, "Anda yakin ingin menghapus tugas '%s'?", task->taskName);
            if (!getConfirmation(confirmMsg)) {
                printf("Penghapusan tugas dibatalkan.\n");
                return;
            }
            printf("Menghapus tugas: %s\n", task->taskName);
            deleteTask(project, task->taskId, 0);
            break;

        case BATCH_STATUS_CHANGE: {
            TaskStatus oldStatus = task->status;
            snprintf(confirmMsg, MAX_DESC_LEN, "Anda yakin ingin mengubah status tugas '%s' dari %s ke %s?",
                    task->taskName, taskStatusToString[oldStatus], 
                    taskStatusToString[operation->data.statusChange.newStatus]);
            if (!getConfirmation(confirmMsg)) {
                printf("Perubahan status dibatalkan.\n");
                return;
            }
            task->status = operation->data.statusChange.newStatus;
            printf("Mengubah status tugas '%s' dari %s ke %s\n",
                   task->taskName,
                   taskStatusToString[oldStatus],
                   taskStatusToString[task->status]);
            recordTaskStatusChange(task->taskId, oldStatus, task->status, "SYSTEM");
            break;
        }

        case BATCH_EDIT:
            snprintf(confirmMsg, MAX_DESC_LEN, "Anda yakin ingin mengubah detail tugas '%s'?", task->taskName);
            if (!getConfirmation(confirmMsg)) {
                printf("Perubahan detail dibatalkan.\n");
                return;
            }
            if (strlen(operation->data.editItem.taskName) > 0) {
                strncpy(task->taskName, operation->data.editItem.taskName, MAX_NAME_LEN - 1);
            }
            if (strlen(operation->data.editItem.description) > 0) {
                strncpy(task->description, operation->data.editItem.description, MAX_DESC_LEN - 1);
            }
            if (strlen(operation->data.editItem.dueDate) > 0) {
                strncpy(task->dueDate, operation->data.editItem.dueDate, DATE_LEN - 1);
            }
            printf("Mengubah detail tugas: %s\n", task->taskName);
            break;
    }
}

void processBatchDeleteTasks(Project* project) {
    if (!project) {
        printf("Proyek tidak valid.\n");
        return;
    }

    initBatchQueue();

    printf("\n=== BATCH DELETE TASKS ===\n");
    printf("Masukkan ID tugas yang ingin dihapus.\n");
    printf("Ketik 'done' untuk selesai dan memproses penghapusan.\n");
    printf("Ketik 'cancel' untuk membatalkan operasi.\n\n");
    displayProjectWBS(project);

    char input[MAX_ID_LEN];
    while (1) {
        printf("\n MASUKAN ID Tugas / (ketik 'done' untuk selesai, 'cancel' untuk batal): ");
        fgets(input, MAX_ID_LEN, stdin);
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "done") == 0) break;
        if (strcmp(input, "cancel") == 0) {
            printf("Operasi batch dibatalkan.\n");
            freeQueue(batch_task_queue);
            batch_task_queue = NULL;
            return;
        }

        Task* task = findTaskInProjectById(project, input);
        if (task) {
            BatchOperation* operation = (BatchOperation*)malloc(sizeof(BatchOperation));
            if (!operation) {
                printf("Gagal mengalokasi memori untuk operasi batch.\n");
                continue;
            }
            operation->type = BATCH_DELETE;
            strncpy(operation->data.deleteItem.taskId, input, MAX_ID_LEN - 1);
            enqueue(batch_task_queue, operation);
            printf("Tugas '%s' ditambahkan ke antrian batch.\n", task->taskName);
        } else {
            printf("Tugas ID '%s' tidak ditemukan.\n", input);
        }
    }

    if (isQueueEmpty(batch_task_queue)) {
        printf("Tidak ada tugas untuk dihapus.\n");
        freeQueue(batch_task_queue);
        batch_task_queue = NULL;
        return;
    }

    printf("\nMenghapus tugas-tugas batch...\n");
    int count = 0;
    while (!isQueueEmpty(batch_task_queue)) {
        BatchOperation* operation = (BatchOperation*)dequeue(batch_task_queue);
        if (operation) {
            processBatchOperation(project, operation);
            free(operation);
            count++;
        }
    }
    printf("%d tugas berhasil dihapus.\n", count);

    freeQueue(batch_task_queue);
    batch_task_queue = NULL;
}

void processBatchStatusChange(Project* project) {
    if (!project) {
        printf("Proyek tidak valid.\n");
        return;
    }

    initBatchQueue();

    printf("\n=== BATCH STATUS CHANGE ===\n");
    printf("Masukkan ID tugas dan status baru.\n");
    printf("Ketik 'done' untuk selesai dan memproses perubahan.\n");
    printf("Ketik 'cancel' untuk membatalkan operasi.\n\n");
    displayProjectWBS(project);

    char input[MAX_ID_LEN];
    while (1) {
        printf("\nMASUKAN ID Tugas / (ketik 'done' untuk selesai, 'cancel' untuk batal): ");
        fgets(input, MAX_ID_LEN, stdin);
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "done") == 0) break;
        if (strcmp(input, "cancel") == 0) {
            printf("Operasi batch dibatalkan.\n");
            freeQueue(batch_task_queue);
            batch_task_queue = NULL;
            return;
        }

        Task* task = findTaskInProjectById(project, input);
        if (task) {
            printf("Status saat ini: %s\n", taskStatusToString[task->status]);
            TaskStatus newStatus = getTaskStatusFromInput();

            BatchOperation* operation = (BatchOperation*)malloc(sizeof(BatchOperation));
            if (!operation) {
                printf("Gagal mengalokasi memori untuk operasi batch.\n");
                continue;
            }
            operation->type = BATCH_STATUS_CHANGE;
            strncpy(operation->data.statusChange.taskId, input, MAX_ID_LEN - 1);
            operation->data.statusChange.newStatus = newStatus;
            enqueue(batch_task_queue, operation);
            printf("Perubahan status tugas '%s' ditambahkan ke antrian batch.\n", task->taskName);
        } else {
            printf("Tugas ID '%s' tidak ditemukan.\n", input);
        }
    }

    if (isQueueEmpty(batch_task_queue)) {
        printf("Tidak ada perubahan status untuk diproses.\n");
        freeQueue(batch_task_queue);
        batch_task_queue = NULL;
        return;
    }

    printf("\nMemproses perubahan status batch...\n");
    int count = 0;
    while (!isQueueEmpty(batch_task_queue)) {
        BatchOperation* operation = (BatchOperation*)dequeue(batch_task_queue);
        if (operation) {
            processBatchOperation(project, operation);
            free(operation);
            count++;
        }
    }
    printf("%d perubahan status berhasil diproses.\n", count);

    freeQueue(batch_task_queue);
    batch_task_queue = NULL;
}

void processBatchEdit(Project* project) {
    if (!project) {
        printf("Proyek tidak valid.\n");
        return;
    }

    initBatchQueue();

    printf("\n=== BATCH EDIT TASKS ===\n");
    printf("Masukkan ID tugas untuk edit.\n");
    printf("Ketik 'done' untuk selesai dan memproses perubahan.\n");
    printf("Ketik 'cancel' untuk membatalkan operasi.\n\n");
    displayProjectWBS(project);

    char input[MAX_ID_LEN];
    char buffer[MAX_DESC_LEN];
    while (1) {
        printf("\nID Tugas (ketik 'done' untuk selesai, 'cancel' untuk batal): ");
        fgets(input, MAX_ID_LEN, stdin);
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "done") == 0) break;
        if (strcmp(input, "cancel") == 0) {
            printf("Operasi batch dibatalkan.\n");
            freeQueue(batch_task_queue);
            batch_task_queue = NULL;
            return;
        }

        Task* task = findTaskInProjectById(project, input);
        if (task) {
            BatchOperation* operation = (BatchOperation*)malloc(sizeof(BatchOperation));
            if (!operation) {
                printf("Gagal mengalokasi memori untuk operasi batch.\n");
                continue;
            }
            operation->type = BATCH_EDIT;
            strncpy(operation->data.editItem.taskId, input, MAX_ID_LEN - 1);

            printf("Nama baru (kosongkan jika tidak ingin ubah): ");
            fgets(buffer, MAX_NAME_LEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            strncpy(operation->data.editItem.taskName, buffer, MAX_NAME_LEN - 1);

            printf("Deskripsi baru (kosongkan jika tidak ingin ubah): ");
            fgets(buffer, MAX_DESC_LEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            strncpy(operation->data.editItem.description, buffer, MAX_DESC_LEN - 1);

            printf("Tenggat baru (YYYY-MM-DD, kosongkan jika tidak ingin ubah): ");
            fgets(buffer, DATE_LEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            strncpy(operation->data.editItem.dueDate, buffer, DATE_LEN - 1);

            enqueue(batch_task_queue, operation);
            printf("Edit tugas '%s' ditambahkan ke antrian batch.\n", task->taskName);
        } else {
            printf("Tugas ID '%s' tidak ditemukan.\n", input);
        }
    }

    if (isQueueEmpty(batch_task_queue)) {
        printf("Tidak ada perubahan untuk diproses.\n");
        freeQueue(batch_task_queue);
        batch_task_queue = NULL;
        return;
    }

    printf("\nMemproses perubahan batch...\n");
    int count = 0;
    while (!isQueueEmpty(batch_task_queue)) {
        BatchOperation* operation = (BatchOperation*)dequeue(batch_task_queue);
        if (operation) {
            processBatchOperation(project, operation);
            free(operation);
            count++;
        }
    }
    printf("%d perubahan berhasil diproses.\n", count);

    freeQueue(batch_task_queue);
    batch_task_queue = NULL;
} 