#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "task.h"

// Global variables
long id_counter = 0;

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

char* generateUniqueId(const char* prefix) {
    static long simple_counter_for_id = 0;
    static char id_buffer[MAX_ID_LEN];
    
    if (id_counter == 0 && simple_counter_for_id == 0) {
        id_counter = (long)time(NULL);
    }
    
    simple_counter_for_id++;
    snprintf(id_buffer, MAX_ID_LEN, "%s%ld", prefix, simple_counter_for_id);
    return id_buffer;
}

int getSubMenuChoice(int max_option) {
    int choice;
    printf("Pilihan: ");
    scanf("%d", &choice);
    clearBuffer(); // Consume newline
    
    if (choice >= 0 && choice <= max_option) {
        return choice;
    }
    
    printf("Pilihan tidak valid.\n");
    return -1;
}

void searchTasksByName(Task* root, const char* searchTerm, int* foundCount, int level) {
    if (!root || !searchTerm) return;

    if (strstr(root->taskName, searchTerm) != NULL ||
        strstr(root->description, searchTerm) != NULL) {
        for (int i = 0; i < level; i++) printf("  ");
        printf("- %s (ID: %s)\n", root->taskName, root->taskId);
        (*foundCount)++;
    }

    // Search in children
    Task* child = root->firstChild;
    while (child) {
        searchTasksByName(child, searchTerm, foundCount, level + 1);
        child = child->nextSibling;
    }
}

void findAndPrintTasksByStatus(Task* root, TaskStatus status, int* foundCount, int level) {
    if (!root) return;

    if (root->status == status) {
        for (int i = 0; i < level; i++) printf("  ");
        printf("- %s (ID: %s)\n", root->taskName, root->taskId);
        (*foundCount)++;
    }

    // Search in children
    Task* child = root->firstChild;
    while (child) {
        findAndPrintTasksByStatus(child, status, foundCount, level + 1);
        child = child->nextSibling;
    }
}

int getConfirmation(const char* message) {
    char input[10];
    printf("\n%s (y/n): ", message);
    fgets(input, sizeof(input), stdin);
    return (input[0] == 'y' || input[0] == 'Y');
} 