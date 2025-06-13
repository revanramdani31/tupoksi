#include "project.h"
#include "task.h"
#include "utils.h"
#include "stack.h"
#include "undo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern Stack* undo_stack;
extern const char* taskStatusToString[];

// Global variables
Project** projects = NULL;
int projectCount = 0;
int projectCapacity = 0;
#define INITIAL_CAPACITY 10

// Global stack for change log
static Stack* changeHistoryStack = NULL;

void initProjectArray() {
    projectCapacity = INITIAL_CAPACITY;
    projects = (Project**)malloc(sizeof(Project*) * projectCapacity);
    if (!projects) {
        perror("Gagal alokasi array proyek");
        exit(EXIT_FAILURE);
    }
    projectCount = 0;
}

void expandProjectArray() {
    int newCapacity = projectCapacity * 2;
    Project** newProjects = (Project**)realloc(projects, sizeof(Project*) * newCapacity);
    if (!newProjects) {
        perror("Gagal ekspansi array proyek");
        return;
    }
    projects = newProjects;
    projectCapacity = newCapacity;
}

void addProject(Project* project) {
    if (projectCount >= projectCapacity) {
        expandProjectArray();
    }
    if (projectCount < projectCapacity) {
        projects[projectCount++] = project;
    }
}

void removeProject(Project* project) {
    int found = -1;
    for (int i = 0; i < projectCount; i++) {
        if (projects[i] == project) {
            found = i;
            break;
        }
    }
    
    if (found >= 0) {
        for (int i = found; i < projectCount - 1; i++) {
            projects[i] = projects[i + 1];
        }
        projectCount--;
    }
}

void freeProjectArray() {
    if (projects) {
        for (int i = 0; i < projectCount; i++) {
            if (projects[i]) {
                if (projects[i]->rootTasks) {
                    deepFreeTask(projects[i]->rootTasks);
                }
                free(projects[i]);
            }
        }
        free(projects);
        projects = NULL;
    }
    projectCount = 0;
    projectCapacity = 0;
}

Project* createProjectInternal(const char* id, const char* name) {
    Project* newProject = (Project*)malloc(sizeof(Project));
    if (!newProject) {
        perror("Gagal alokasi Project");
        return NULL;
    }

    strncpy(newProject->projectId, id, MAX_ID_LEN - 1);
    newProject->projectId[MAX_ID_LEN - 1] = '\0';

    strncpy(newProject->projectName, name, MAX_NAME_LEN - 1);
    newProject->projectName[MAX_NAME_LEN - 1] = '\0';
    
    newProject->rootTasks = NULL;
    return newProject;
}

Project* createProject(const char* name) {
    if (!name) return NULL;

    char new_id[MAX_ID_LEN];
    strncpy(new_id, generateUniqueId("PRJ"), MAX_ID_LEN-1);
    new_id[MAX_ID_LEN-1] = '\0';

    Project* newProject = createProjectInternal(new_id, name);
    if (!newProject) return NULL;

    // Resize project array if needed
    if (projectCount >= projectCapacity) {
        int newCapacity = projectCapacity == 0 ? 10 : projectCapacity * 2;
        Project** newProjects = (Project**)realloc(projects, newCapacity * sizeof(Project*));
        if (!newProjects) {
            free(newProject);
            return NULL;
        }
        projects = newProjects;
        projectCapacity = newCapacity;
    }

    // Add project to array
    projects[projectCount++] = newProject;

    // Record undo action
    UndoAction* action = (UndoAction*)malloc(sizeof(UndoAction));
    if (action) {
        action->type = UNDO_PROJECT_CREATION;
        strncpy(action->projectId, newProject->projectId, MAX_ID_LEN - 1);
        pushUndoAction(undo_stack, action);
    }

    // Record change
    recordChange("Proyek baru dibuat", "SYSTEM", "PROJECT_CREATION");

    printf("Proyek berhasil dibuat dengan ID: %s\n", newProject->projectId);
    return newProject;
}

Project* findProjectById(const char* projectId) {
    if (!projectId) return NULL;

    for (int i = 0; i < projectCount; i++) {
        if (strcmp(projects[i]->projectId, projectId) == 0) {
            return projects[i];
        }
    }
    return NULL;
}

void editProjectDetails(Project* project) {
    if (!project) return;

    char buffer[MAX_NAME_LEN];

    printf("\nEdit Proyek ID: %s\n", project->projectId);
    printf("Nama saat ini: %s\n", project->projectName);
    printf("Masukkan nama baru (kosongkan jika tidak ingin ubah): ");
    fgets(buffer, MAX_NAME_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) {
        strncpy(project->projectName, buffer, MAX_NAME_LEN - 1);
        project->projectName[MAX_NAME_LEN - 1] = '\0';
        recordChange("Nama proyek diubah", "SYSTEM", "PROJECT_UPDATE");
    }

    printf("Proyek berhasil diperbarui.\n");
}

void deleteProject(const char* projectId) {
    if (!projectId) return;

    int index = -1;
    for (int i = 0; i < projectCount; i++) {
        if (strcmp(projects[i]->projectId, projectId) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Proyek tidak ditemukan.\n");
        return;
    }

    // Free all tasks in the project
    Task* root = projects[index]->rootTasks;
    while (root) {
        Task* next = root->nextSibling;
        deepFreeTask(root);
        root = next;
    }

    // Record change before freeing
    recordChange("Proyek dihapus", "SYSTEM", "PROJECT_DELETION");

    // Free the project
    free(projects[index]);

    // Shift remaining projects
    for (int i = index; i < projectCount - 1; i++) {
        projects[i] = projects[i + 1];
    }
    projectCount--;

    printf("Proyek berhasil dihapus.\n");
}

void listAllProjects() {
    if (projectCount == 0) {
        printf("Tidak ada proyek.\n");
        return;
    }

    printf("\nDaftar Proyek:\n");
    printf("----------------------------------------\n");
    printf("ID\t\tNama Proyek\t\tJumlah Tugas\n");
    printf("----------------------------------------\n");

    for (int i = 0; i < projectCount; i++) {
        int taskCount = 0;
        Task* root = projects[i]->rootTasks;
        while (root) {
            taskCount++;
            root = root->nextSibling;
        }

        printf("%s\t%s\t\t%d\n",
               projects[i]->projectId,
               projects[i]->projectName,
               taskCount);
    }
    printf("----------------------------------------\n");
}

void displayProjectWBS(Project* project) {
    if (!project) return;

    printf("\nWork Breakdown Structure - %s [%s]:\n",
           project->projectName, project->projectId);
    printf("----------------------------------------\n");

    Task* root = project->rootTasks;
    while (root) {
        displayWBSTree(root, 0, root->nextSibling == NULL);
        root = root->nextSibling;
    }
    printf("----------------------------------------\n");
}

void displayUpcomingTasks(Project* project) {
    if (!project) return;

    printf("\nTugas yang Akan Datang - %s [%s]:\n",
           project->projectName, project->projectId);
    printf("----------------------------------------\n");
    printf("ID\t\tNama\t\tStatus\t\tTenggat\n");
    printf("----------------------------------------\n");

    time_t now = time(NULL);
    struct tm* current = localtime(&now);
    char today[DATE_LEN];
    strftime(today, DATE_LEN, "%Y-%m-%d", current);

    Task* root = project->rootTasks;
    while (root) {
        if (strlen(root->dueDate) > 0 && strcmp(root->dueDate, today) >= 0) {
            printf("%s\t%s\t%s\t%s\n",
                   root->taskId,
                   root->taskName,
                   taskStatusToString[root->status],
                   root->dueDate);
        }
        root = root->nextSibling;
    }
    printf("----------------------------------------\n");
}

void searchProjectsByName(const char* searchTerm) {
    printf("\n--- Hasil Pencarian Proyek untuk '%s' ---\n", searchTerm);
    int found_count = 0;
    
    for (int i = 0; i < projectCount; i++) {
        if (strstr(projects[i]->projectName, searchTerm) != NULL) {
            printf("- %s (ID: %s)\n", projects[i]->projectName, projects[i]->projectId);
            found_count++;
        }
    }
    
    if (found_count == 0) {
        printf("Tidak ada proyek yang cocok.\n");
    }
    printf("-----------------------------------------\n");
}

void searchTasksInProject(Project* project, const char* searchTerm) {
    if (!project) {
        printf("ERROR: Proyek tidak valid untuk pencarian.\n");
        return;
    }
    
    printf("\n--- Hasil Pencarian Tugas '%s' di Proyek '%s' ---\n",
           searchTerm, project->projectName);
    int found_count = 0;
    searchTasksByName(project->rootTasks, searchTerm, &found_count, 0);
    
    if (found_count == 0) {
        printf("Tidak ada tugas yang cocok.\n");
    }
    printf("---------------------------------------------------------\n");
}

void reportTasksByStatus(Project* project) {
    if (!project) {
        printf("ERROR: Proyek tidak valid.\n");
        return;
    }
    
    printf("--- Laporan Tugas berdasarkan Status di Proyek: %s ---\n", project->projectName);
    TaskStatus selected_status = getTaskStatusFromInput();
    printf("--- Tugas Status: %s ---\n", taskStatusToString[selected_status]);
    
    int found_count = 0;
    findAndPrintTasksByStatus(project->rootTasks, selected_status, &found_count, 0);
    
    if (found_count == 0) {
        printf("Tidak ada tugas status '%s'.\n", taskStatusToString[selected_status]);
    }
    printf("-----------------------------------\n");
}

void recordChange(const char* description, const char* userId, const char* changeType) {
    if (!description || !userId || !changeType) return;

    if (!changeHistoryStack) {
        changeHistoryStack = createStack();
        if (!changeHistoryStack) {
            printf("ERROR: Gagal membuat change history stack.\n");
            return;
        }
    }

    // Create change record
    char timestamp[DATE_LEN];
    time_t now = time(NULL);
    strftime(timestamp, DATE_LEN, "%Y-%m-%d", localtime(&now));

    // Format the change record
    char record[MAX_DESC_LEN];
    snprintf(record, MAX_DESC_LEN, "%s | %s | %s | %s",
             timestamp, changeType, description, userId);

    // Push to stack
    UndoAction* action = (UndoAction*)malloc(sizeof(UndoAction));
    if (action) {
        action->type = UNDO_TASK_CREATION; // Use any type, we only care about the record
        strncpy(action->taskId, record, MAX_ID_LEN - 1);
        action->taskId[MAX_ID_LEN - 1] = '\0';
        pushUndoAction(changeHistoryStack, action);
    }
}

void displayChangeLog() {
    if (!changeHistoryStack || isEmpty(changeHistoryStack)) {
        printf("Tidak ada log perubahan.\n");
        return;
    }

    printf("\nLog Perubahan:\n");
    printf("----------------------------------------\n");
    printf("Tanggal\t\tTipe\t\tDeskripsi\t\tUser\n");
    printf("----------------------------------------\n");

    Stack* tempStack = createStack();
    if (!tempStack) return;

    while (!isEmpty(changeHistoryStack)) {
        UndoAction* action = popUndoAction(changeHistoryStack);
        if (action) {
            printf("%s\n", action->taskId);
            pushUndoAction(tempStack, action);
        }
    }

    while (!isEmpty(tempStack)) {
        UndoAction* action = popUndoAction(tempStack);
        pushUndoAction(changeHistoryStack, action);
    }

    freeStack(tempStack);
    printf("----------------------------------------\n");
}

void searchChangeLog(const char* searchTerm) {
    if (!searchTerm || !changeHistoryStack || isEmpty(changeHistoryStack)) {
        printf("Tidak ada log perubahan.\n");
        return;
    }

    printf("\nHasil Pencarian Log Perubahan:\n");
    printf("----------------------------------------\n");
    printf("Tanggal\t\tTipe\t\tDeskripsi\t\tUser\n");
    printf("----------------------------------------\n");

    Stack* tempStack = createStack();
    if (!tempStack) return;

    int found = 0;
    while (!isEmpty(changeHistoryStack)) {
        UndoAction* action = popUndoAction(changeHistoryStack);
        if (action && strstr(action->taskId, searchTerm) != NULL) {
            printf("%s\n", action->taskId);
            found++;
        }
        pushUndoAction(tempStack, action);
    }

    while (!isEmpty(tempStack)) {
        UndoAction* action = popUndoAction(tempStack);
        pushUndoAction(changeHistoryStack, action);
    }

    freeStack(tempStack);
    printf("----------------------------------------\n");
    printf("Ditemukan %d hasil.\n", found);
}

void exportChangeLogToCSV() {
    if (!changeHistoryStack || isEmpty(changeHistoryStack)) {
        printf("Tidak ada log perubahan untuk diekspor.\n");
        return;
    }

    FILE* file = fopen("change_log.csv", "w");
    if (!file) {
        printf("Gagal membuat file CSV.\n");
        return;
    }

    // Write CSV header
    fprintf(file, "Tanggal,Tipe,Deskripsi,User\n");

    Stack* tempStack = createStack();
    if (!tempStack) {
        fclose(file);
        return;
    }

    while (!isEmpty(changeHistoryStack)) {
        UndoAction* action = popUndoAction(changeHistoryStack);
        if (action) {
            fprintf(file, "%s\n", action->taskId);
            pushUndoAction(tempStack, action);
        }
    }

    while (!isEmpty(tempStack)) {
        UndoAction* action = popUndoAction(tempStack);
        pushUndoAction(changeHistoryStack, action);
    }

    freeStack(tempStack);
    fclose(file);
    printf("Log perubahan berhasil diekspor ke change_log.csv\n");
} 