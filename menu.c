#include "menu.h"
#include "project.h"
#include "task.h"
#include "batch.h"
#include "undo.h"
#include "ileio.h"
#include "utils.h"

void displayMainMenu() {
    printf("\n=== MENU UTAMA ===\n");
    printf("1. Kelola Proyek\n");
    printf("2. Kelola Tugas\n");
    printf("3. Laporan\n");
    printf("4. Undo\n");
    printf("5. Batch Delete\n");
    printf("6. Simpan Data\n");
    printf("7. Muat Data\n");
    printf("0. Keluar\n");
    printf("Pilihan: ");
}

void displayProjectMenu() {
    printf("\n=== MENU PROYEK ===\n");
    printf("1. Buat Proyek Baru\n");
    printf("2. Edit Proyek\n");
    printf("3. Hapus Proyek\n");
    printf("4. Lihat Semua Proyek\n");
    printf("5. Cari Proyek\n");
    printf("6. Lihat WBS Proyek\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void displayTaskMenu() {
    printf("\n=== MENU TUGAS ===\n");
    printf("1. Buat Tugas Baru\n");
    printf("2. Tambah Sub-Tugas\n");
    printf("3. Hapus Tugas\n");
    printf("4. Cari Tugas\n");
    printf("5. Edit Tugas\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void displayReportMenu() {
    printf("\n=== MENU LAPORAN ===\n");
    printf("1. Laporan Tugas Berdasarkan Status\n");
    printf("2. Laporan Tugas Berdasarkan Prioritas\n");
    printf("0. Kembali\n");
    printf("Pilihan: ");
}

void handleProjectMenu() {
    int choice;
    char projectId[MAX_ID_LEN];
    Project* project;
    
    do {
        displayProjectMenu();
        scanf("%d", &choice);
        getchar(); // Consume newline
        
        switch (choice) {
            case 1: {
                char projectName[MAX_NAME_LEN];
                printf("Masukkan nama proyek: ");
                fgets(projectName, MAX_NAME_LEN, stdin);
                projectName[strcspn(projectName, "\n")] = 0;
                createProject(projectName);
                break;
            }
            case 2: {
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
            }
            case 3: {
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                deleteProject(projectId);
                break;
            }
            case 4:
                listAllProjects();
                break;
            case 5: {
                char searchName[MAX_NAME_LEN];
                printf("Masukkan nama proyek: ");
                fgets(searchName, MAX_NAME_LEN, stdin);
                searchName[strcspn(searchName, "\n")] = 0;
                searchProjectsByName(searchName);
                break;
            }
            case 6: {
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
            }
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
    Project* project;
    Task* task;
    
    do {
        displayTaskMenu();
        scanf("%d", &choice);
        getchar(); // Consume newline
        
        switch (choice) {
            case 1: {
                printf("\nDaftar Proyek yang Tersedia:\n");
                listAllProjects();
                printf("\nMasukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    char taskName[MAX_NAME_LEN];
                    char taskDesc[MAX_DESC_LEN];
                    char dueDate[DATE_LEN];
                    
                    printf("Masukkan nama tugas: ");
                    fgets(taskName, MAX_NAME_LEN, stdin);
                    taskName[strcspn(taskName, "\n")] = 0;
                    
                    printf("Masukkan deskripsi tugas: ");
                    fgets(taskDesc, MAX_DESC_LEN, stdin);
                    taskDesc[strcspn(taskDesc, "\n")] = 0;
                    
                    TaskStatus status = getTaskStatusFromInput();
                    TaskPriority priority = getTaskPriorityFromInput();
                    
                    printf("Masukkan tanggal tenggat (YYYY-MM-DD, kosongkan jika tidak ada): ");
                    fgets(dueDate, DATE_LEN, stdin);
                    dueDate[strcspn(dueDate, "\n")] = 0;
                    
                    Task* newTask = createTaskAndRecordUndo(taskName, taskDesc, project, NULL, status, priority, dueDate);
                    if (newTask) {
                        addRootTaskToProject(project, newTask);
                    }
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
            case 2: {
                printf("\nDaftar Proyek yang Tersedia:\n");
                listAllProjects();
                printf("\nMasukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    printf("\nStruktur Tugas dalam Proyek:\n");
                    displayProjectWBS(project);
                    printf("\nMasukkan ID tugas parent: ");
                    fgets(taskId, MAX_ID_LEN, stdin);
                    taskId[strcspn(taskId, "\n")] = 0;
                    task = findTaskInProjectById(project, taskId);
                    if (task) {
                        char taskName[MAX_NAME_LEN];
                        char taskDesc[MAX_DESC_LEN];
                        char dueDate[DATE_LEN];
                        
                        printf("Masukkan nama tugas: ");
                        fgets(taskName, MAX_NAME_LEN, stdin);
                        taskName[strcspn(taskName, "\n")] = 0;
                        
                        printf("Masukkan deskripsi tugas: ");
                        fgets(taskDesc, MAX_DESC_LEN, stdin);
                        taskDesc[strcspn(taskDesc, "\n")] = 0;
                        
                        TaskStatus status = getTaskStatusFromInput();
                        TaskPriority priority = getTaskPriorityFromInput();
                        
                        printf("Masukkan tanggal tenggat (YYYY-MM-DD, kosongkan jika tidak ada): ");
                        fgets(dueDate, DATE_LEN, stdin);
                        dueDate[strcspn(dueDate, "\n")] = 0;
                        
                        Task* newTask = createTaskAndRecordUndo(taskName, taskDesc, project, task, status, priority, dueDate);
                        if (newTask) {
                            addChildTask(task, newTask);
                        }
                    } else {
                        printf("Tugas tidak ditemukan.\n");
                    }
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
            case 3: {
                printf("\nDaftar Proyek yang Tersedia:\n");
                listAllProjects();
                printf("\nMasukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    printf("\nStruktur Tugas dalam Proyek:\n");
                    displayProjectWBS(project);
                    printf("\nMasukkan ID tugas: ");
                    fgets(taskId, MAX_ID_LEN, stdin);
                    taskId[strcspn(taskId, "\n")] = 0;
                    deleteTask(project, taskId, 0);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
            case 4: {
                printf("\nDaftar Proyek yang Tersedia:\n");
                listAllProjects();
                printf("\nMasukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    char searchName[MAX_NAME_LEN];
                    printf("Masukkan nama tugas: ");
                    fgets(searchName, MAX_NAME_LEN, stdin);
                    searchName[strcspn(searchName, "\n")] = 0;
                    searchTasksInProjectByName(project, searchName);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
            case 5: {
                printf("\nDaftar Proyek yang Tersedia:\n");
                listAllProjects();
                printf("\nMasukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    printf("\nStruktur Tugas dalam Proyek:\n");
                    displayProjectWBS(project);
                    printf("\nMasukkan ID tugas: ");
                    fgets(taskId, MAX_ID_LEN, stdin);
                    taskId[strcspn(taskId, "\n")] = 0;
                    task = findTaskInProjectById(project, taskId);
                    if (task) {
                        editTask(task);
                    } else {
                        printf("Tugas tidak ditemukan.\n");
                    }
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
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
    
    do {
        displayReportMenu();
        scanf("%d", &choice);
        getchar(); // Consume newline
        
        switch (choice) {
            case 1: {
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    reportTasksByStatus(project);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
            case 2: {
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    reportTasksByPriority(project);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
            case 0:
                return;
            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
}

void runMainMenu() {
    int choice;
    char projectId[MAX_ID_LEN];
    Project* project;
    
    do {
        displayMainMenu();
        scanf("%d", &choice);
        getchar(); // Consume newline
        
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
                processUndoLastTaskCreation();
                break;
            case 5: {
                printf("Masukkan ID proyek: ");
                fgets(projectId, MAX_ID_LEN, stdin);
                projectId[strcspn(projectId, "\n")] = 0;
                project = findProjectById(projectId);
                if (project) {
                    processBatchDeleteTasks(project);
                } else {
                    printf("Proyek tidak ditemukan.\n");
                }
                break;
            }
            case 6:
                saveDataToFile("data.txt");
                break;
            case 7:
                loadDataFromFile("data.txt");
                break;
            case 0:
                printf("Terima kasih telah menggunakan program ini.\n");
                return;
            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (1);
} 