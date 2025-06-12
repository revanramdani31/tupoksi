#include "utils.h"
#include "task.h"
#include "project.h"
#include "linkedlist.h"
#include "stack.h"
#include "queue.h"
#include "undo.h"
#include "batch.h"
#include "ileio.h"
#include "menu.h"

// Global variables
extern LinkedListNode* project_list_head;
extern Stack* undo_stack;
extern Queue* batch_task_queue;
extern int id_counter;  // Add external declaration for id_counter

// String arrays for task status and priority
extern const char* taskStatusToString[];
extern const char* taskPriorityToString[];

int main() {
    srand(time(NULL)); 
    id_counter = 0; // Let generateUniqueId initialize with time

    undo_stack = createStack(MAX_UNDO_ACTIONS);
    if (!undo_stack) {
        printf("Gagal menginisialisasi undo stack.\n");
        return 1;
    }
    
    batch_task_queue = createQueue();
    if (!batch_task_queue) {
        printf("Gagal menginisialisasi batch queue.\n");
        freeStack(undo_stack);
        return 1;
    }
    
    loadDataFromFile(DATA_FILE); 

    // Run main menu
    runMainMenu();
    
    // Cleanup
    if (undo_stack) {
        freeStack(undo_stack);
    }
    if (batch_task_queue) {
        freeQueue(batch_task_queue);
    }
    
    // Free all projects and their tasks
    LinkedListNode* node = project_list_head;
    while (node) {
        Project* project = (Project*)node->data;
        if (project) {
            LinkedListNode* task_node = project->tasks_head;
            while (task_node) {
                Task* task = (Task*)task_node->data;
                if (task) {
                    deepFreeTask(task);
                }
                task_node = task_node->next;
            }
            free(project);
        }
        LinkedListNode* temp = node;
        node = node->next;
        free(temp->data);
        free(temp);
    }
    
    return 0;
}