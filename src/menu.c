#include "menu.h"
#include "utils.h"
#include "project.h"
#include "task.h"
#include "batch.h"
#include "undo.h"
#include "ileio.h"
#include "stack.h"

extern Stack* undo_stack;
extern const char* taskStatusToString[];

void displayMainMenu() {
    printf("\n=== MENU UTAMA ===\n");
    printf("1. Proyek\n");
    printf("2. Tugas\n");
    printf("3. Laporan\n");
    printf("4. Batch\n");
    printf("5. File\n");
    printf("6. Logs\n");
    printf("0. Keluar\n");
    printf("Pilihan: ");
}

void displayProjectMenu() {
    printf("\n=== MENU PROYEK ===\n");
    printf("1. Buat Proyek Baru\n");
    printf("2. Lihat Semua Proyek\n");
    printf("3. Edit Proyek\n");
    printf("4. Hapus Proyek\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void displayTaskMenu() {
    printf("\n=== MENU TUGAS ===\n");
    printf("1. Buat Tugas Baru\n");
    printf("2. Lihat Struktur Tugas\n");
    printf("3. Edit Tugas\n");
    printf("4. Hapus Tugas\n");
    printf("5. Cari Tugas\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void displayReportMenu() {
    printf("\n=== MENU LAPORAN ===\n");
    printf("1. Lihat Tugas Berdasarkan Status\n");
    printf("2. Lihat Tugas yang Akan Datang\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void displayLogMenu() {
    printf("\n=== MENU LOGS ===\n");
    printf("1. Lihat Riwayat Tugas\n");
    printf("2. Analisis Status Tugas\n");
    printf("3. Lihat Log Perubahan\n");
    printf("4. Cari Log\n");
    printf("5. Export Log ke CSV\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void displayBatchMenu() {
    printf("\n=== MENU BATCH ===\n");
    printf("1. Batch Hapus Tugas\n");
    printf("2. Batch Ubah Status\n");
    printf("3. Batch Edit Tugas\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void displayFileMenu() {
    printf("\n=== MENU FILE ===\n");
    printf("1. Simpan Data\n");
    printf("2. Muat Data\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void handleProjectMenu() {
    int choice;
    char name[MAX_NAME_LEN];
    char projectId[MAX_ID_LEN];
    Project* project;

    do {
        displayProjectMenu();
        scanf("%d", &choice);
        clearBuffer();

        switch (choice) {
            case 1:
                printf("Masukkan nama proyek: ");
                fgets(name, MAX_NAME_LEN, stdin);
                name[strcspn(name, "\n")] = 0;
                createProject(name);
                break;

            case 2:
                listAllProjects();
                break;

            case 3:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    editProjectDetails(project);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;

            case 4:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    deleteProject(projectId);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;

            case 0:
                return;

            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
}

void handleTaskMenu() {
    int choice;
    char projectId[MAX_ID_LEN];
    char taskId[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char desc[MAX_DESC_LEN];
    char dueDate[DATE_LEN];
    Project* project;
    Task* parentTask = NULL;
    Task* task;

    do {
        displayTaskMenu();
        scanf("%d", &choice);
        clearBuffer();

        switch (choice) {
            case 1:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (!project) {
                    printf("Proyek tidak ditemukan.\n");
                    break;
                }

                printf("Masukkan ID tugas induk (kosongkan untuk tugas root): ");
                fgets(taskId, MAX_ID_LEN, stdin);
                taskId[strcspn(taskId, "\n")] = 0;
                if (strlen(taskId) > 0) {
                    parentTask = findTaskInProjectById(project, taskId);
                    if (!parentTask) {
                        printf("Tugas induk tidak ditemukan.\n");
                        break;
                    }
                }

                printf("Masukkan nama tugas: ");
                fgets(name, MAX_NAME_LEN, stdin);
                name[strcspn(name, "\n")] = 0;

                printf("Masukkan deskripsi tugas: ");
                fgets(desc, MAX_DESC_LEN, stdin);
                desc[strcspn(desc, "\n")] = 0;

                printf("Masukkan tenggat (YYYY-MM-DD): ");
                fgets(dueDate, DATE_LEN, stdin);
                dueDate[strcspn(dueDate, "\n")] = 0;

                TaskStatus status = getTaskStatusFromInput();
                createTaskAndRecordUndo(name, desc, project, parentTask, status, dueDate);
                break;

            case 2:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    displayWBSTree(project->rootTasks, 0, 1);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;

            case 3:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (!project) {
                    printf("Proyek tidak ditemukan.\n");
                    break;
                }

                printf("Masukkan ID tugas: ");
                fgets(taskId, MAX_ID_LEN, stdin);
                taskId[strcspn(taskId, "\n")] = 0;
                task = findTaskInProjectById(project, taskId);
                if (task) {
                    editTask(task);
                } else {
                    printf("Tugas tidak ditemukan.\n");
                }
                break;

            case 4:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (!project) {
                    printf("Proyek tidak ditemukan.\n");
                    break;
                }

                printf("Masukkan ID tugas: ");
                fgets(taskId, MAX_ID_LEN, stdin);
                taskId[strcspn(taskId, "\n")] = 0;
                deleteTask(project, taskId, 0);
                break;

            case 5:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (!project) {
                    printf("Proyek tidak ditemukan.\n");
                    break;
                }

                printf("Masukkan kata kunci pencarian: ");
                fgets(name, MAX_NAME_LEN, stdin);
                name[strcspn(name, "\n")] = 0;
                int count = 0;
                displayTasksBySearchTerm(project->rootTasks, name, 0, &count);
                printf("\nTotal %d tugas ditemukan.\n", count);
                break;

            case 0:
                return;

            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
}

void handleReportMenu() {
    int choice;
    char projectId[MAX_ID_LEN];
    Project* project;
    int count;

    do {
        displayReportMenu();
        scanf("%d", &choice);
        clearBuffer();

        switch (choice) {
            case 1:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (!project) {
                    printf("Proyek tidak ditemukan.\n");
                    break;
                }

                printf("\nPilih status tugas:\n");
                for (int i = 0; i < TASK_STATUS_COUNT; i++) {
                    printf("%d. %s\n", i + 1, taskStatusToString[i]);
                }
                printf("Pilihan: ");
                int statusChoice;
                scanf("%d", &statusChoice);
                while(getchar() != '\n');
                if (statusChoice > 0 && statusChoice <= TASK_STATUS_COUNT) {
                    count = 0;
                    displayTasksByStatus(project->rootTasks, statusChoice - 1, 0, &count);
                    printf("\nTotal %d tugas dengan status %s.\n", 
                           count, taskStatusToString[statusChoice - 1]);
                } else {
                    printf("Pilihan status tidak valid.\n");
                }
                break;

            case 2:
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    displayUpcomingTasks(project);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;

            case 0:
                return;

            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
}

void handleLogMenu() {
    int choice;
    char taskId[MAX_ID_LEN];

    do {
        displayLogMenu();
        scanf("%d", &choice);
        clearBuffer();

        switch (choice) {
            case 1:
                printf("Masukkan ID tugas: ");
                fgets(taskId, MAX_ID_LEN, stdin);
                taskId[strcspn(taskId, "\n")] = 0;
                displayTaskHistory(taskId);
                break;

            case 2:
                printf("Masukkan ID tugas: ");
                fgets(taskId, MAX_ID_LEN, stdin);
                taskId[strcspn(taskId, "\n")] = 0;
                analyzeTaskStatusChanges(taskId);
                break;

            case 3:
                displayChangeLog();
                break;

            case 4:
                printf("Masukkan kata kunci pencarian: ");
                char searchTerm[MAX_NAME_LEN];
                fgets(searchTerm, MAX_NAME_LEN, stdin);
                searchTerm[strcspn(searchTerm, "\n")] = 0;
                searchChangeLog(searchTerm);
                break;

            case 5:
                exportChangeLogToCSV();
                break;

            case 0:
                return;

            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
}

void handleBatchMenu() {
    int choice;
    char projectId[MAX_ID_LEN];
    Project* project;

    do {
        displayBatchMenu();
        scanf("%d", &choice);
        clearBuffer();

        if (choice == 0) break;

        printf("Masukkan ID proyek: ");
        fgets(projectId, MAX_ID_LEN, stdin);
        projectId[strcspn(projectId, "\n")] = 0;
        project = findProjectById(projectId);

        if (!project) {
            printf("Proyek tidak ditemukan.\n");
            continue;
        }

        switch (choice) {
            case 1:
                processBatchDeleteTasks(project);
                break;
            case 2:
                processBatchStatusChange(project);
                break;
            case 3:
                processBatchEdit(project);
                break;
            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
}

void handleFileMenu() {
    int choice;
    char filename[MAX_NAME_LEN];

    do {
        displayFileMenu();
        scanf("%d", &choice);
        clearBuffer();

        switch (choice) {
            case 1:
                printf("Masukkan nama file untuk menyimpan: ");
                fgets(filename, MAX_NAME_LEN, stdin);
                filename[strcspn(filename, "\n")] = 0;
                saveDataToFile(filename);
                break;

            case 2:
                printf("Masukkan nama file untuk dimuat: ");
                fgets(filename, MAX_NAME_LEN, stdin);
                filename[strcspn(filename, "\n")] = 0;
                loadDataFromFile(filename);
                break;

            case 0:
                return;

            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
}

void handleUndo() {
    if (isEmpty(undo_stack)) {
        printf("Tidak ada aksi yang dapat di-undo.\n");
        return;
    }

    UndoAction* action = popUndoAction(undo_stack);
    if (!action) {
        printf("Gagal melakukan undo.\n");
        return;
    }

    switch (action->type) {
        case UNDO_TASK_CREATION:
            deleteTask(findProjectById(action->projectId), action->taskId, 1);
            break;

        case UNDO_TASK_DELETION:
            // Implement task restoration if needed
            break;

        case UNDO_PROJECT_CREATION:
            deleteProject(action->projectId);
            break;

        case UNDO_PROJECT_DELETION:
            // Implement project restoration if needed
            break;

        default:
            printf("Tipe aksi undo tidak valid.\n");
    }

    free(action);
    printf("Aksi berhasil di-undo.\n");
}

void runMainMenu() {
    int choice;

    do {
        displayMainMenu();
        scanf("%d", &choice);
        clearBuffer();

        switch (choice) {
            case 1:
                handleProjectMenu();
                break;
            case 2:
                handleTaskMenu();
                break;
            case 3:
                handleReportMenu();
                break;
            case 4:
                handleBatchMenu();
                break;
            case 5:
                handleFileMenu();
                break;
            case 6:
                handleLogMenu();
                break;
            case 0:
                printf("Thank you for using Project Management System!\n");
                break;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 0);
} 