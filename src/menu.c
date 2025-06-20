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
    printf("\n");
    printf("+================================================+\n");
    printf("|              MENU UTAMA SISTEM                  |\n");
    printf("+================================================+\n");
    printf("| 1. Proyek                                       |\n");
    printf("| 2. Tugas                                        |\n");
    printf("| 3. Undo tambah tugas                            |\n");
    printf("| 4. Batch                                        |\n");
    printf("| 5. File                                         |\n");
    printf("| 6. Logs                                         |\n");
    printf("| 0. Keluar                                       |\n");
    printf("+================================================+\n");
    printf("Pilihan: ");
}

void displayProjectMenu() {
    printf("\n");
    printf("+================================================+\n");
    printf("|                 MENU PROYEK                     |\n");
    printf("+================================================+\n");
    printf("| 1. Buat Proyek Baru                            |\n");
    printf("| 2. Lihat Semua Proyek                          |\n");
    printf("| 3. Edit Proyek                                 |\n");
    printf("| 4. Hapus Proyek                                |\n");
    printf("| 0. Kembali                                     |\n");
    printf("+================================================+\n");
    printf("Pilihan: ");
}

void displayTaskMenu() {
    printf("\n");
    printf("+================================================+\n");
    printf("|                 MENU TUGAS                      |\n");
    printf("+================================================+\n");
    printf("| 1. Buat Tugas Baru                             |\n");
    printf("| 2. Lihat Struktur Tugas (WBS)                  |\n");
    printf("| 3. Edit Tugas                                  |\n");
    printf("| 4. Hapus Tugas                                 |\n");
    printf("| 5. Cari Tugas                                  |\n");
    printf("| 6. Lihat Antrian Tugas Siap Dikerjakan         |\n");
    printf("| 7. Selesaikan Tugas Berikutnya dari Antrian    |\n");
    printf("| 0. Kembali                                     |\n");
    printf("+================================================+\n");
    printf("Pilihan: ");
}


void displayLogMenu() {
    printf("\n");
    printf("+================================================+\n");
    printf("|                  MENU LOGS                      |\n");
    printf("+================================================+\n");
    printf("| 1. Lihat Riwayat Tugas                         |\n");
    printf("| 2. Analisis Status Tugas                       |\n");
    printf("| 3. Lihat Log Perubahan                         |\n");
    printf("| 4. Export Log ke CSV                           |\n");
    printf("| 0. Kembali                                     |\n");
    printf("+================================================+\n");
    printf("Pilihan: ");
}

void displayBatchMenu() {
    printf("\n");
    printf("+================================================+\n");
    printf("|                 MENU BATCH                      |\n");
    printf("+================================================+\n");
    printf("| 1. Batch Hapus Tugas                           |\n");
    printf("| 2. Batch Ubah Status                           |\n");
    printf("| 3. Batch Edit Tugas                            |\n");
    printf("| 0. Kembali                                     |\n");
    printf("+================================================+\n");
    printf("Pilihan: ");
}

void displayFileMenu() {
    printf("\n");
    printf("+================================================+\n");
    printf("|                 MENU FILE                       |\n");
    printf("+================================================+\n");
    printf("| 1. Simpan Data                                 |\n");
    printf("| 2. Muat Data                                   |\n");
    printf("| 0. Kembali                                     |\n");
    printf("+================================================+\n");
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

                // Show current task structure first
                printf("\nStruktur Tugas Saat Ini:\n");
                displayProjectWBS(project);

                printf("\nMasukkan ID tugas induk (kosongkan untuk tugas root): ");
                fgets(taskId, MAX_ID_LEN, stdin);
                taskId[strcspn(taskId, "\n")] = 0;
                if (strlen(taskId) > 0) {
                    parentTask = findTaskInProjectById(project, taskId);
                    if (!parentTask) {
                        printf("Error: Tugas induk dengan ID '%s' tidak ditemukan.\n", taskId);
                        printf("Pastikan ID tugas induk valid dan ada dalam struktur tugas di atas.\n");
                        break;
                    }
                    // Check if parent task is cancelled
                    if (parentTask->status == TASK_STATUS_DIBATALKAN) {
                        printf("Error: Tidak dapat menambahkan sub-tugas ke tugas yang sudah dibatalkan.\n");
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
                    displayProjectWBS(project);
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
            case 6:
            	displayCompletionQueue();
            	break;
        	case 7: // <-- TAMBAHKAN CASE BARU
           	 processNextTaskInQueue();
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
                handleUndo();
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
