#include "project.h"
#include "task.h"
#include "menu.h"
#include "utils.h"
#include "undo.h"
#include "stack.h"
#include "ileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void freeAllData() {
    // Free all projects and their tasks
    for (int i = 0; i < projectCount; i++) {
        if (projects[i]) {
            if (projects[i]->rootTasks) {
                deepFreeTask(projects[i]->rootTasks);
            }
            free(projects[i]);
        }
    }
    free(projects);
    projectCount = 0;
    projectCapacity = 0;
}

void initializeSystem() {
    // Initialize project array
    initProjectArray();
    
    // Initialize undo stack
    if (!undo_stack) {
        undo_stack = createStack();
        if (!undo_stack) {
            printf("ERROR: Gagal inisialisasi undo stack.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Load existing data if any
    loadDataFromFile(DATA_FILE);
}

void cleanupSystem() {
    // Save data before exit
    saveDataToFile(DATA_FILE);
    
    // Cleanup undo stack
    if (undo_stack) {
        freeStackAndActions(undo_stack);
        undo_stack = NULL;
    }
    
    // Cleanup project array
    freeProjectArray();
}

void displayWelcomeMessage() {
    printf("\n");
    printf("+================================================+\n");
    printf("|                                                 |\n");
    printf("|         SISTEM MANAJEMEN PROYEK DAN TUGAS      |\n");
    printf("|                                                 |\n");
    printf("|  Fitur:                                        |\n");
    printf("|  - Manajemen Proyek dan Tugas                  |\n");
    printf("|  - Work Breakdown Structure (WBS)              |\n");
    printf("|  - Pelacakan Status dan Riwayat               |\n");
    printf("|  - Pencarian dan Pelaporan                     |\n");
    printf("|                                                 |\n");
    printf("+================================================+\n\n");
}

int main() {
    // Initialize system
    initializeSystem();
    
    // Display welcome message
    displayWelcomeMessage();
    
    // Run main menu
    runMainMenu();
    
    // Cleanup before exit
    cleanupSystem();
    
    return 0;
}