#include "undo.h"
#include "project.h"
#include "task.h"
#include "stack.h"

// Global variables
Stack* undo_stack = NULL;

void processUndoLastTaskCreation() {
    if (isStackEmpty(undo_stack)) {
        printf("Tidak ada aksi yang dapat di-undo.\n");
        return;
    }
    
    UndoAction* action = popUndoAction(undo_stack);
    if (!action) {
        printf("Gagal mengambil aksi undo.\n");
        return;
    }
    
    if (action->type == UNDO_TASK_CREATION) {
        Project* project = findProjectById(action->projectId);
        if (project) {
            deleteTask(project, action->taskId, 1);
            printf("Tugas berhasil di-undo.\n");
        } else {
            printf("Proyek tidak ditemukan.\n");
        }
    } else {
        printf("Tipe aksi undo tidak valid.\n");
    }
    
    free(action);
} 