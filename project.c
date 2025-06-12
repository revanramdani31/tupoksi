#include "project.h"
#include "task.h"
#include "linkedlist.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variable definition
LinkedListNode* project_list_head = NULL;

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
    
    newProject->tasks_head = NULL;
    return newProject;
}

Project* createProject(const char* name) {
    char new_id[MAX_ID_LEN] = {0};
    strncpy(new_id, generateUniqueId("PRJ"), MAX_ID_LEN-1);
    new_id[MAX_ID_LEN-1] = '\0';
    
    Project* proj = createProjectInternal(new_id, name);
    if (proj) {
        appendToList(&project_list_head, proj);
        printf("Proyek '%s' (ID: %s) berhasil dibuat.\n", proj->projectName, proj->projectId);
    } else {
        printf("ERROR: Gagal membuat proyek '%s'.\n", name);
    }
    return proj;
}

Project* findProjectById(const char* projectId) {
    LinkedListNode* current = project_list_head;
    while (current != NULL) {
        Project* p = (Project*)current->data;
        if (strcmp(p->projectId, projectId) == 0) return p;
        current = current->next;
    }
    return NULL;
}

void editProjectDetails(Project* project) {
    if (!project) {
        printf("ERROR: Proyek tidak valid untuk diedit.\n");
        return;
    }
    
    char name_buffer[MAX_NAME_LEN];
    printf("Nama Proyek saat ini: %s (ID: %s)\n", project->projectName, project->projectId);
    printf("Masukkan Nama Proyek baru (kosongkan jika tidak ingin ubah): ");
    fgets(name_buffer, MAX_NAME_LEN, stdin);
    name_buffer[strcspn(name_buffer, "\n")] = 0;
    
    if (strlen(name_buffer) > 0) {
        strncpy(project->projectName, name_buffer, MAX_NAME_LEN - 1);
        project->projectName[MAX_NAME_LEN - 1] = '\0';
        printf("Nama proyek berhasil diubah menjadi '%s'.\n", project->projectName);
    } else {
        printf("Nama proyek tidak diubah.\n");
    }
}

void deleteProject(const char* projectId) {
    Project* project_to_delete = findProjectById(projectId);
    if (!project_to_delete) {
        printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", projectId);
        return;
    }
    
    LinkedListNode* current_task_node = project_to_delete->tasks_head;
    LinkedListNode* next_task_node;
    
    while (current_task_node != NULL) {
        next_task_node = current_task_node->next;
        deepFreeTask((Task*)current_task_node->data);
        free(current_task_node);
        current_task_node = next_task_node;
    }
    
    project_to_delete->tasks_head = NULL;
    removeFromList(&project_list_head, project_to_delete);
    
    char projectNameCopy[MAX_NAME_LEN];
    strncpy(projectNameCopy, project_to_delete->projectName, MAX_NAME_LEN - 1);
    projectNameCopy[MAX_NAME_LEN - 1] = '\0';
    
    free(project_to_delete);
    printf("Proyek '%s' (ID: %s) dihapus.\n", projectNameCopy, projectId);
}

void listAllProjects() {
    printf("\n--- Daftar Proyek ---\n");
    if (project_list_head == NULL) {
        printf("Belum ada proyek.\n");
        printf("---------------------\n");
        return;
    }
    
    int count = 1;
    LinkedListNode* current = project_list_head;
    while (current != NULL) {
        Project* p = (Project*)current->data;
        printf("%d. %s (ID: %s)\n", count++, p->projectName, p->projectId);
        current = current->next;
    }
    printf("---------------------\n");
}

void displayTaskWBS(Task* task, int level) {
    if (!task) return;
    
    for (int i = 0; i < level; ++i) printf("  ");
    printf("|- [%s] %s (Status: %s, Prioritas: %s, Due: %s)\n",
           task->taskId, task->taskName, taskStatusToString[task->status],
           taskPriorityToString[task->priority],
           strlen(task->dueDate) > 0 ? task->dueDate : "N/A");
    
    for (int i = 0; i < level + 1; ++i) printf("  ");
    printf("   Desc: %s\n", task->description);
    
    LinkedListNode* child_node = task->children_head;
    while (child_node != NULL) {
        displayTaskWBS((Task*)child_node->data, level + 1);
        child_node = child_node->next;
    }
}

void displayProjectWBS(Project* project) {
    if (!project) {
        printf("Proyek tidak ditemukan untuk WBS.\n");
        return;
    }
    
    printf("\n--- WBS Proyek: %s (ID: %s) ---\n", project->projectName, project->projectId);
    if (project->tasks_head == NULL) {
        printf("Belum ada tugas.\n");
    }
    
    LinkedListNode* task_node = project->tasks_head;
    while (task_node != NULL) {
        displayTaskWBS((Task*)task_node->data, 0);
        task_node = task_node->next;
    }
    printf("---------------------------------\n");
}

void searchProjectsByName(const char* searchTerm) {
    printf("\n--- Hasil Pencarian Proyek untuk '%s' ---\n", searchTerm);
    int found_count = 0;
    LinkedListNode* current = project_list_head;
    
    while (current != NULL) {
        Project* p = (Project*)current->data;
        if (strstr(p->projectName, searchTerm) != NULL) {
            printf("- %s (ID: %s)\n", p->projectName, p->projectId);
            found_count++;
        }
        current = current->next;
    }
    
    if (found_count == 0) {
        printf("Tidak ada proyek yang cocok.\n");
    }
    printf("-----------------------------------------\n");
}

void searchTasksInProjectRecursive(LinkedListNode* task_node_head, const char* searchTerm,
                                 int* found_count, int level) {
    LinkedListNode* current_node = task_node_head;
    while (current_node != NULL) {
        Task* task = (Task*)current_node->data;
        if (strstr(task->taskName, searchTerm) != NULL ||
            strstr(task->description, searchTerm) != NULL) {
            for (int i = 0; i < level; ++i) printf("  ");
            printf("  |- [%s] %s (Status: %s, Prioritas: %s, Due: %s)\n",
                   task->taskId, task->taskName, taskStatusToString[task->status],
                   taskPriorityToString[task->priority],
                   strlen(task->dueDate) > 0 ? task->dueDate : "N/A");
            (*found_count)++;
        }
        if (task->children_head) {
            searchTasksInProjectRecursive(task->children_head, searchTerm, found_count, level + 1);
        }
        current_node = current_node->next;
    }
}

void searchTasksInProjectByName(Project* project, const char* searchTerm) {
    if (!project) {
        printf("ERROR: Proyek tidak valid untuk pencarian.\n");
        return;
    }
    
    printf("\n--- Hasil Pencarian Tugas '%s' di Proyek '%s' ---\n",
           searchTerm, project->projectName);
    int found_count = 0;
    searchTasksInProjectRecursive(project->tasks_head, searchTerm, &found_count, 0);
    
    if (found_count == 0) {
        printf("Tidak ada tugas yang cocok.\n");
    }
    printf("---------------------------------------------------------\n");
}

void findAndPrintTasksByStatusRecursive(LinkedListNode* task_head, TaskStatus status_to_find,
                                      int* count, int level) {
    LinkedListNode* current = task_head;
    while(current) {
        Task* t = (Task*)current->data;
        if (t->status == status_to_find) {
            for (int i = 0; i < level; ++i) printf("  ");
            printf("  |- [%s] %s (Prioritas: %s, Due: %s)\n",
                   t->taskId, t->taskName,
                   taskPriorityToString[t->priority],
                   strlen(t->dueDate) > 0 ? t->dueDate : "N/A");
            (*count)++;
        }
        if (t->children_head) {
            findAndPrintTasksByStatusRecursive(t->children_head, status_to_find, count, level + 1);
        }
        current = current->next;
    }
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
    findAndPrintTasksByStatusRecursive(project->tasks_head, selected_status, &found_count, 0);
    
    if (found_count == 0) {
        printf("Tidak ada tugas status '%s'.\n", taskStatusToString[selected_status]);
    }
    printf("-----------------------------------\n");
}

void findAndPrintTasksByPriorityRecursive(LinkedListNode* task_head, TaskPriority prio_to_find,
                                        int* count, int level) {
    LinkedListNode* current = task_head;
    while(current) {
        Task* t = (Task*)current->data;
        if (t->priority == prio_to_find) {
            for (int i = 0; i < level; ++i) printf("  ");
            printf("  |- [%s] %s (Status: %s, Due: %s)\n",
                   t->taskId, t->taskName,
                   taskStatusToString[t->status],
                   strlen(t->dueDate) > 0 ? t->dueDate : "N/A");
            (*count)++;
        }
        if (t->children_head) {
            findAndPrintTasksByPriorityRecursive(t->children_head, prio_to_find, count, level + 1);
        }
        current = current->next;
    }
}

void reportTasksByPriority(Project* project) {
    if (!project) {
        printf("ERROR: Proyek tidak valid.\n");
        return;
    }
    
    printf("--- Laporan Tugas per Prioritas di Proyek: %s ---\n", project->projectName);
    TaskPriority selected_priority = getTaskPriorityFromInput();
    printf("--- Tugas Prioritas: %s ---\n", taskPriorityToString[selected_priority]);
    
    int found_count = 0;
    findAndPrintTasksByPriorityRecursive(project->tasks_head, selected_priority, &found_count, 0);
    
    if (found_count == 0) {
        printf("Tidak ada tugas prioritas '%s'.\n", taskPriorityToString[selected_priority]);
    }
    printf("-----------------------------------\n");
} 