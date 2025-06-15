#include "ileio.h"
#include "project.h"
#include "task.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void saveTasksOfProject(FILE* file_ptr, Task* task) {
    if (!file_ptr || !task) return;

    // Save this task
    fprintf(file_ptr, "T,%s,%s,%s,%s,%s,%d,%s\n",
            task->taskId, task->projectId,
            (task->parent ? task->parent->taskId : "NULL"),
            task->taskName, task->description,
            (int)task->status,
            task->dueDate[0] == '\0' ? "NULL" : task->dueDate);
    
    // Save children recursively
    Task* child = task->firstChild;
    while (child) {
        saveTasksOfProject(file_ptr, child);
        child = child->nextSibling;
    }
}

void saveDataToFile(const char* filename) {
	int i;
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/%s", filename);
    
    FILE* fp = fopen(filepath, "w");
    if (!fp) {
        printf("ERROR: Gagal buka file '%s' untuk simpan.\n", filepath);
        return;
    }
    
    for (i = 0; i < projectCount; i++) { 
        Project* project = projects[i];
        fprintf(fp, "P,%s,%s\n", project->projectId, project->projectName);
        
        // Save all root tasks and their children
        Task* rootTask = project->rootTasks;
        while (rootTask) {
            saveTasksOfProject(fp, rootTask);
            rootTask = rootTask->nextSibling;
        }
    }
    
    fclose(fp);
    printf("Data disimpan ke %s.\n", filepath);
}

void buildTaskHierarchyForProject(Project* project, Task** all_tasks, int task_count) {
	int i,j;
    if (!project || !all_tasks || task_count == 0) return;
    
    // First pass: Set up parent-child relationships
    for (i = 0; i < task_count; i++) { 
        Task* task = all_tasks[i];
        if (strlen(task->parentTaskId) == 0 || strcmp(task->parentTaskId, "NULL") == 0) {
            // This is a root task
            if (!project->rootTasks) {
                project->rootTasks = task;
            } else {
                Task* current = project->rootTasks;
                while (current->nextSibling) {
                    current = current->nextSibling;
                }
                current->nextSibling = task;
            }
        } else {
            // Find parent task
            Task* parentTask = NULL;
            for (j = 0; j < task_count; j++) {
                if (strcmp(all_tasks[j]->taskId, task->parentTaskId) == 0) {
                    parentTask = all_tasks[j];
                    break;
                }
            }
            
            if (parentTask) {
                task->parent = parentTask;
                if (!parentTask->firstChild) {
                    parentTask->firstChild = task;
                } else {
                    Task* current = parentTask->firstChild;
                    while (current->nextSibling) {
                        current = current->nextSibling;
                    }
                    current->nextSibling = task;
                }
            } else {
                printf("WARNING: Parent ID '%s' untuk task '%s' tidak ditemukan.\n",
                       task->parentTaskId, task->taskName);
                // Add as root task
                if (!project->rootTasks) {
                    project->rootTasks = task;
                } else {
                    Task* current = project->rootTasks;
                    while (current->nextSibling) {
                        current = current->nextSibling;
                    }
                    current->nextSibling = task;
                }
            }
        }
    }
}

void loadDataFromFile(const char* filename) {
	int i;
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "data/%s", filename);
    
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        printf("INFO: File '%s' tidak ditemukan.\n", filepath);
        return;
    }
    
    char line[MAX_LINE_LEN];
    Project* current_loading_project = NULL;
    Task** temp_tasks = NULL;
    int task_count = 0;
    int task_capacity = 10;
    
    if (current_loading_project && task_count > 0) {
        buildTaskHierarchyForProject(current_loading_project, temp_tasks, task_count);
        printf("Mempopulasi ulang antrian tugas...\n");
        repopulateCompletionQueue(current_loading_project->rootTasks);
    }
    // Initialize temporary task array
    temp_tasks = (Task**)malloc(sizeof(Task*) * task_capacity);
    if (!temp_tasks) {
        printf("ERROR: Gagal alokasi memori untuk array tugas.\n");
        fclose(fp);
        return;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        char* type = strtok(line, ",");
        if (!type) continue;
        
        if (strcmp(type, "P") == 0) {
            if (current_loading_project && task_count > 0) {
                buildTaskHierarchyForProject(current_loading_project, temp_tasks, task_count);
                // Reset task array
                for (i = 0; i < task_count; i++) {
                    temp_tasks[i] = NULL;
                }
                task_count = 0;
            }
            
            char* id = strtok(NULL, ",");
            char* name = strtok(NULL, "\n");
            
            if (id && name) {
                current_loading_project = createProjectInternal(id, name);
                if (current_loading_project) {
                    addProject(current_loading_project);
                } else {
                    printf("ERROR: Gagal load proyek: ID=%s.\n", id);
                }
            }
        } else if (strcmp(type, "T") == 0) {
            char* task_id_str = strtok(NULL, ",");
            char* project_id_str = strtok(NULL, ",");
            char* parent_task_id_str = strtok(NULL, ",");
            char* task_name_str = strtok(NULL, ",");
            char* task_desc_str = strtok(NULL, ",");
            char* status_val_str = strtok(NULL, ",");
            char* due_date_val_str = strtok(NULL, "\n");
            
            if (task_id_str && project_id_str && parent_task_id_str && task_name_str &&
                task_desc_str && status_val_str && due_date_val_str) {
                
                if (strcmp(project_id_str, current_loading_project->projectId) != 0) {
                    printf("WARNING: Task %s, project ID %s beda dari %s. Dilewati.\n",
                           task_id_str, project_id_str, current_loading_project->projectId);
                    continue;
                }
                
                TaskStatus status = (TaskStatus)atoi(status_val_str);
                
                Task* task = createTaskInternal(task_id_str, task_name_str, task_desc_str,
                                              project_id_str, parent_task_id_str,
                                              status,
                                              (strcmp(due_date_val_str, "NULL") == 0 ? "" : due_date_val_str));
                
                if (task) {
                    // Expand task array if needed
                    if (task_count >= task_capacity) {
                        task_capacity *= 2;
                        Task** new_temp_tasks = (Task**)realloc(temp_tasks, sizeof(Task*) * task_capacity);
                        if (!new_temp_tasks) {
                            printf("ERROR: Gagal ekspansi array tugas.\n");
                            free(task);
                            continue;
                        }
                        temp_tasks = new_temp_tasks;
                    }
                    temp_tasks[task_count++] = task;
                } else {
                    printf("ERROR: Gagal load tugas: ID=%s.\n", task_id_str);
                }
            }
        }
    }
    
    // Process last project if any
    if (current_loading_project && task_count > 0) {
        buildTaskHierarchyForProject(current_loading_project, temp_tasks, task_count);
    }
    
    // Cleanup
    free(temp_tasks);
    fclose(fp);
    printf("Data dimuat dari %s.\n", filepath);
}
