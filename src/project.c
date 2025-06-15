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

Project** projects = NULL;
int projectCount = 0;
int projectCapacity = 0;
#define INITIAL_CAPACITY 10

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
    strncpy(new_id, generateProjectId("PRJ"), MAX_ID_LEN-1);
    new_id[MAX_ID_LEN-1] = '\0';

    Project* newProject = createProjectInternal(new_id, name);
    if (!newProject) return NULL;

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

    projects[projectCount++] = newProject;

    UndoAction* action = (UndoAction*)malloc(sizeof(UndoAction));
    if (action) {
        action->type = UNDO_PROJECT_CREATION;
        strncpy(action->projectId, newProject->projectId, MAX_ID_LEN - 1);
        pushUndoAction(undo_stack, action);
    }

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

    Task* root = projects[index]->rootTasks;
    while (root) {
        Task* next = root->nextSibling;
        deepFreeTask(root);
        root = next;
    }

    recordChange("Proyek dihapus", "SYSTEM", "PROJECT_DELETION");
    free(projects[index]);

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

    printf("\n");
    printf("*=======================================================*\n");
    printf("||                STRUKTUR RINCIAN KERJA               ||\n");
    printf("||              (Work Breakdown Structure)             ||\n");
    printf("*=======================================================*\n");
    printf("||  Proyek: %-44s||\n", project->projectName);
    printf("||  ID    : %-44s||\n", project->projectId);
    printf("*=======================================================*\n");
    printf("||  Keterangan Status:                                 ||\n");
    printf("||  [ ] = Baru         [~] = Dalam Proses             ||\n");
    printf("||  [*] = Selesai      [X] = Dibatalkan              ||\n");
    printf("*=======================================================*\n");
    printf("||  Format: [Status] Nama Tugas (ID, Tenggat)         ||\n");
    printf("*=======================================================*\n\n");

    int totalTasks = 0;
    int statusCounts[TASK_STATUS_COUNT] = {0};
    countTasksAndStatus(project->rootTasks, &totalTasks, statusCounts);

    printf(".:[ Statistik Tugas ]:..\n");
    printf("  +----------------------+\n");
    printf("  | Total Tugas   : %-4d|\n", totalTasks);
    printf("  | Baru          : %-4d|\n", statusCounts[TASK_STATUS_BARU]);
    printf("  | Dalam Proses  : %-4d|\n", statusCounts[TASK_STATUS_DALAM_PROSES]);
    printf("  | Selesai       : %-4d|\n", statusCounts[TASK_STATUS_SELESAI]);
    printf("  | Dibatalkan    : %-4d|\n", statusCounts[TASK_STATUS_DIBATALKAN]);
    printf("  +----------------------+\n\n");

    printf(".:[ Struktur Tugas ]:...\n");
    Task* root = project->rootTasks;
    while (root) {
        displayWBSTree(root, 0, root->nextSibling == NULL);
        root = root->nextSibling;
    }
    printf("\n");
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

    ChangeLog* newLog = (ChangeLog*)malloc(sizeof(ChangeLog));
    if (!newLog) {
        printf("ERROR: Gagal alokasi memori untuk log.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);

    strftime(newLog->timestamp, DATE_LEN, "%Y-%m-%d", timeinfo);
    strncpy(newLog->changeId, generateChangeId("CHG"), MAX_ID_LEN - 1);
    strncpy(newLog->description, description, MAX_DESC_LEN - 1);
    strncpy(newLog->userId, userId, MAX_ID_LEN - 1);
    strncpy(newLog->changeType, changeType, MAX_NAME_LEN - 1);
    
    newLog->changeId[MAX_ID_LEN - 1] = '\0';
    newLog->description[MAX_DESC_LEN - 1] = '\0';
    newLog->timestamp[DATE_LEN - 1] = '\0';
    newLog->userId[MAX_ID_LEN - 1] = '\0';
    newLog->changeType[MAX_NAME_LEN - 1] = '\0';

    push(changeHistoryStack, newLog);
    
}
void displayChangeLog() {
    // Asumsi stack global Anda untuk log perubahan bernama changeHistoryStack
    if (!changeHistoryStack || isEmpty(changeHistoryStack)) {
        printf("Log Perubahan kosong.\n");
        return;
    }

    // Buat stack sementara agar tidak merusak stack asli
    Stack* tempStack = createStack();
    if (!tempStack) {
        printf("ERROR: Gagal membuat stack sementara.\n");
        return;
    }

    // Cetak Header Tabel
    printf("Log Perubahan:\n");
    printf("+========+============+==================+==========================================+==========+\n");
    printf("| ID     | Tanggal    | Tipe             | Deskripsi                                | User     |\n");
    printf("+========+============+==================+==========================================+==========+\n");

    // Pindahkan semua item ke stack sementara sambil mencetaknya
    while (!isEmpty(changeHistoryStack)) {
        // Ambil data dari stack utama
        ChangeLog* log = (ChangeLog*)pop(changeHistoryStack);
        if (log) {
            // Cetak data langsung dari field struct, tidak perlu strtok!
            printf("| %-6s | %-10s | %-16s | %-40s | %-8s |\n",
                   log->changeId,
                   log->timestamp,
                   log->changeType,
                   log->description,
                   log->userId);
            // Masukkan ke stack sementara untuk menjaga urutan
            push(tempStack, log);
        }
    }
    printf("+========+============+==================+==========================================+==========+\n");

    // Kembalikan semua item dari stack sementara ke stack utama
    while (!isEmpty(tempStack)) {
        push(changeHistoryStack, pop(tempStack));
    }

    // Hapus stack sementara dari memori
    free(tempStack);
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
