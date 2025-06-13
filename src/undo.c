#include "undo.h"
#include "project.h"
#include "task.h"
#include "stack.h"
#include <stdio.h>

// Global variables
Stack* undo_stack = NULL;

void pushUndoAction(Stack* s, UndoAction* action) {
    if (!s || !action) return;
    push(s, action);
}

UndoAction* popUndoAction(Stack* s) {
    if (!s) return NULL;
    return (UndoAction*)pop(s);
}

void freeStackAndActions(Stack* s) {
    if (!s) return;
    while (!isEmpty(s)) {
        UndoAction* action = popUndoAction(s);
        if (action) free(action);
    }
    freeStack(s);
}

void processUndoLastTaskCreation() {
    if (isEmpty(undo_stack)) {
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