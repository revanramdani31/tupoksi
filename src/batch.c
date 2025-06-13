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

void processBatchDeleteTasks(Project* project) {
    if (!project) {
        printf("Proyek tidak valid.\n");
        return;
    }

    if (batch_task_queue) {
        freeQueueAndItems(batch_task_queue);
    }
    batch_task_queue = createQueue();

    printf("Masukkan ID tugas yang ingin dihapus (ketik 'done' untuk selesai):\n");
    displayProjectWBS(project);

    char input[MAX_ID_LEN];
    while (1) {
        printf("ID Tugas: ");
        fgets(input, MAX_ID_LEN, stdin);
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "done") == 0) break;

        Task* task = findTaskInProjectById(project, input);
        if (task) {
            BatchDeleteItem* item = (BatchDeleteItem*)malloc(sizeof(BatchDeleteItem));
            if (!item) {
                printf("Gagal mengalokasi memori untuk item batch.\n");
                continue;
            }
            strncpy(item->taskId, input, MAX_ID_LEN - 1);
            item->taskId[MAX_ID_LEN - 1] = '\0';
            enqueueBatchItem(batch_task_queue, item);
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
        BatchDeleteItem* item = dequeueBatchItem(batch_task_queue);
        if (item) {
            Task* task = findTaskInProjectById(project, item->taskId);
            if (task) {
                printf("Menghapus tugas: %s\n", task->taskName);
                deleteTask(project, item->taskId, 0);
                count++;
            }
            free(item);
        }
    }
    printf("%d tugas berhasil dihapus.\n", count);

    freeQueue(batch_task_queue);
    batch_task_queue = NULL;
} 