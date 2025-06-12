#include "ileio.h"
#include "project.h"
#include "task.h"
#include "linkedlist.h"
#include "utils.h"

void saveTasksOfProject(FILE* file_ptr, LinkedListNode* task_head_node) {
    LinkedListNode* current_task_node = task_head_node;
    while(current_task_node != NULL) {
        Task* task = (Task*)current_task_node->data;
        fprintf(file_ptr, "T,%s,%s,%s,%s,%s,%d,%d,%s\n",
                task->taskId, task->projectId,
                (task->parent ? task->parent->taskId : "NULL"),
                task->taskName, task->description,
                (int)task->status, (int)task->priority,
                task->dueDate[0] == '\0' ? "NULL" : task->dueDate);
        
        if (task->children_head) {
            saveTasksOfProject(file_ptr, task->children_head);
        }
        current_task_node = current_task_node->next;
    }
}

void saveDataToFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("ERROR: Gagal buka file '%s' untuk simpan.\n", filename);
        return;
    }
    
    LinkedListNode* p_node = project_list_head;
    while (p_node != NULL) {
        Project* project = (Project*)p_node->data;
        fprintf(fp, "P,%s,%s\n", project->projectId, project->projectName);
        saveTasksOfProject(fp, project->tasks_head);
        p_node = p_node->next;
    }
    
    fclose(fp);
    printf("Data disimpan ke %s.\n", filename);
}

void buildTaskHierarchyForProject(Project* project, LinkedListNode** all_tasks_for_project_head) {
    if (!project || !all_tasks_for_project_head || !(*all_tasks_for_project_head)) return;
    
    LinkedListNode* current_task_node = *all_tasks_for_project_head;
    while (current_task_node != NULL) {
        Task* task = (Task*)current_task_node->data;
        if (strlen(task->parentTaskId) == 0 || strcmp(task->parentTaskId, "NULL") == 0) {
            appendToList(&(project->tasks_head), task);
            task->parent = NULL;
        } else {
            Task* parentTask = NULL;
            LinkedListNode* search_node = *all_tasks_for_project_head;
            
            while(search_node != NULL) {
                Task* potential_parent = (Task*)search_node->data;
                if (task != potential_parent && strcmp(potential_parent->taskId, task->parentTaskId) == 0) {
                    parentTask = potential_parent;
                    break;
                }
                search_node = search_node->next;
            }
            
            if (parentTask) {
                task->parent = parentTask;
                appendToList(&(parentTask->children_head), task);
            } else {
                printf("WARNING: Parent ID '%s' task '%s' tidak ditemukan.\n",
                       task->parentTaskId, task->taskName);
                appendToList(&(project->tasks_head), task);
                task->parent = NULL;
            }
        }
        current_task_node = current_task_node->next;
    }
}

void loadDataFromFile(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("INFO: File '%s' tidak ditemukan.\n", filename);
        return;
    }
    
    char line[MAX_LINE_LEN];
    Project* current_loading_project = NULL;
    LinkedListNode* temp_task_list_for_current_project = NULL;
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        char* type = strtok(line, ",");
        if (!type) continue;
        
        if (strcmp(type, "P") == 0) {
            if (current_loading_project && temp_task_list_for_current_project) {
                buildTaskHierarchyForProject(current_loading_project, &temp_task_list_for_current_project);
                LinkedListNode* temp_node_to_free;
                while(temp_task_list_for_current_project) {
                    temp_node_to_free = temp_task_list_for_current_project;
                    temp_task_list_for_current_project = temp_task_list_for_current_project->next;
                    free(temp_node_to_free);
                }
            }
            
            temp_task_list_for_current_project = NULL;
            char* id = strtok(NULL, ",");
            char* name = strtok(NULL, "\n");
            
            if (id && name) {
                current_loading_project = createProjectInternal(id, name);
                if (current_loading_project) {
                    appendToList(&project_list_head, current_loading_project);
                } else {
                    printf("ERROR: Gagal load proyek: ID=%s.\n", id);
                }
            }
        } else if (strcmp(type, "T") == 0 && current_loading_project) {
            char* task_id_str = strtok(NULL, ",");
            char* project_id_str = strtok(NULL, ",");
            char* parent_task_id_str = strtok(NULL, ",");
            char* task_name_str = strtok(NULL, ",");
            char* task_desc_str = strtok(NULL, ",");
            char* status_val_str = strtok(NULL, ",");
            char* priority_val_str = strtok(NULL, ",");
            char* due_date_val_str = strtok(NULL, "\n");
            
            if (task_id_str && project_id_str && parent_task_id_str && task_name_str &&
                task_desc_str && status_val_str && priority_val_str && due_date_val_str) {
                
                if (strcmp(project_id_str, current_loading_project->projectId) != 0) {
                    printf("WARNING: Task %s, project ID %s beda dari %s. Dilewati.\n",
                           task_id_str, project_id_str, current_loading_project->projectId);
                    continue;
                }
                
                TaskStatus status = (TaskStatus)atoi(status_val_str);
                TaskPriority priority = (TaskPriority)atoi(priority_val_str);
                
                Task* task = createTaskInternal(task_id_str, task_name_str, task_desc_str,
                                              project_id_str, parent_task_id_str,
                                              status, priority,
                                              (strcmp(due_date_val_str, "NULL") == 0 ? "" : due_date_val_str));
                
                if (task) {
                    appendToList(&temp_task_list_for_current_project, task);
                } else {
                    printf("ERROR: Gagal load tugas: ID=%s.\n", task_id_str);
                }
            } else {
                printf("ERROR: Format data tugas tidak lengkap di file untuk baris mulai T,%s...\n",
                       task_id_str ? task_id_str : "(ID tidak terbaca)");
            }
        }
    }
    
    if (current_loading_project && temp_task_list_for_current_project) {
        buildTaskHierarchyForProject(current_loading_project, &temp_task_list_for_current_project);
        LinkedListNode* temp_node_to_free;
        while(temp_task_list_for_current_project) {
            temp_node_to_free = temp_task_list_for_current_project;
            temp_task_list_for_current_project = temp_task_list_for_current_project->next;
            free(temp_node_to_free);
        }
    }
    
    fclose(fp);
    printf("Data dimuat dari %s.\n", filename);
} 