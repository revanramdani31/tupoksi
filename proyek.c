#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

// Definisi Konstanta
#define MAX_NAME_LEN 100
#define MAX_DESC_LEN 255
#define MAX_ID_LEN 50
#define MAX_LINE_LEN 1024
#define DATA_FILE "project_data.txt"
#define MAX_UNDO_ACTIONS 10
#define DATE_LEN 11 // YYYY-MM-DD + null terminator

// --- Enum untuk Atribut Tugas Baru ---
typedef enum {
    TASK_STATUS_BARU, TASK_STATUS_DIKERJAKAN, TASK_STATUS_SELESAI, TASK_STATUS_TERTUNDA,
    TASK_STATUS_COUNT
} TaskStatus;
const char* taskStatusToString[] = {"Baru", "Dikerjakan", "Selesai", "Tertunda"};

typedef enum {
    TASK_PRIORITY_RENDAH, TASK_PRIORITY_SEDANG, TASK_PRIORITY_TINGGI, TASK_PRIORITY_MENDESAK,
    TASK_PRIORITY_COUNT
} TaskPriority;
const char* taskPriorityToString[] = {"Rendah", "Sedang", "Tinggi", "Mendesak"};

// --- Forward Declarations untuk Structs ---
typedef struct LinkedListNode LinkedListNode;
typedef struct Task Task;
typedef struct Project Project;
typedef struct Stack Stack;
typedef struct Queue Queue;
typedef struct UndoAction UndoAction;
typedef struct BatchDeleteItem BatchDeleteItem;

// --- Definisi Struktur Dasar ---
struct LinkedListNode { void* data; struct LinkedListNode* next; };

struct Task {
    char taskId[MAX_ID_LEN];
    char taskName[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    char projectId[MAX_ID_LEN];
    char parentTaskId[MAX_ID_LEN];
    Task* parent;
    LinkedListNode* children_head;
    TaskStatus status;
    TaskPriority priority;
    char dueDate[DATE_LEN];
};

struct Project {
    char projectId[MAX_ID_LEN];
    char projectName[MAX_NAME_LEN];
    LinkedListNode* tasks_head;
};

struct Stack {
    LinkedListNode* top;
    int count;
    int limit;
};

struct Queue {
    LinkedListNode* front;
    LinkedListNode* rear;
    int count;
};

typedef enum {
    UNDO_ACTION_NONE,
    UNDO_TASK_CREATION
} UndoActionType;

struct UndoAction {
    UndoActionType type;
    char taskId[MAX_ID_LEN];
    char projectId[MAX_ID_LEN];
};

struct BatchDeleteItem {
    char taskId[MAX_ID_LEN];
};

// --- Variabel Global ---
LinkedListNode* project_list_head = NULL;
long id_counter = 0; // Akan diinisialisasi di main
Stack* undo_stack = NULL;
Queue* batch_task_queue = NULL;

// --- Prototipe Fungsi ---
// Utilitas
LinkedListNode* createLinkedListNode(void* data);
void appendToList(LinkedListNode** head, void* data);
void removeFromList(LinkedListNode** head, void* data_to_remove);
void generateUniqueId(const char* prefix, char* id_buffer);
TaskStatus getTaskStatusFromInput();
TaskPriority getTaskPriorityFromInput();
int getSubMenuChoice(int max_option);

// Stack
Stack* createStack(int limit);
int isStackEmpty(Stack* s);
int isStackFull(Stack* s);
void pushUndoAction(Stack* s, UndoAction* action);
UndoAction* popUndoAction(Stack* s);
void freeStackAndActions(Stack* s);

// Queue
Queue* createQueue();
int isQueueEmpty(Queue* q);
void enqueueBatchItem(Queue* q, BatchDeleteItem* item);
BatchDeleteItem* dequeueBatchItem(Queue* q);
void freeQueueAndItems(Queue* q);
void freeQueue(Queue* q);

// Manajemen Proyek & Tugas
Project* createProjectInternal(const char* id, const char* name);
Project* createProject(const char* name);
Task* createTaskInternal(const char* id, const char* name, const char* desc, const char* projectId, const char* parentIdStr, TaskStatus status, TaskPriority priority, const char* dueDate);
Task* createTaskAndRecordUndo(const char* name, const char* desc, Project* project, Task* parentTaskOpt, TaskStatus status, TaskPriority priority, const char* dueDate);
void addRootTaskToProject(Project* project, Task* task);
void addChildTask(Task* parentTask, Task* childTask);
Project* findProjectById(const char* projectId);
Task* findTaskInProjectRecursive(LinkedListNode* task_node_head, const char* taskId);
Task* findTaskInProjectById(Project* project, const char* taskId);
void listAllProjects();
void displayTaskWBS(Task* task, int level);
void displayProjectWBS(Project* project);

// Edit
void editProjectDetails(Project* project);
void editTaskDetails(Task* task);

// Search
void searchProjectsByName(const char* searchTerm);
void searchTasksInProjectRecursive(LinkedListNode* task_node_head, const char* searchTerm, int* found_count, int level);
void searchTasksInProjectByName(Project* project, const char* searchTerm);

// Report
void findAndPrintTasksByStatusRecursive(LinkedListNode* task_head, TaskStatus status_to_find, int* count, int level);
void reportTasksByStatus(Project* project);
void findAndPrintTasksByPriorityRecursive(LinkedListNode* task_head, TaskPriority prio_to_find, int* count, int level);
void reportTasksByPriority(Project* project);

// Hapus
void deepFreeTask(Task* task);
void deleteTask(Project* project, const char* taskId, int an_undo_action);
void deleteProject(const char* projectId);

// Persistensi Data
void saveTasksOfProject(FILE* file_ptr, LinkedListNode* task_head_node);
void saveDataToFile(const char* filename);
void buildTaskHierarchyForProject(Project* project, LinkedListNode** all_tasks_for_project_head);
void loadDataFromFile(const char* filename);

// Undo & Batch
void processUndoLastTaskCreation();
void processBatchDeleteTasks(Project* project);

// Lainnya
void freeAllMemory();
void displayMenu();

// --- Implementasi Fungsi ---

LinkedListNode* createLinkedListNode(void* data) {
    LinkedListNode* newNode = (LinkedListNode*)malloc(sizeof(LinkedListNode));
    if (!newNode) { perror("Gagal alokasi LinkedListNode"); exit(EXIT_FAILURE); }
    newNode->data = data; newNode->next = NULL; return newNode;
}

void appendToList(LinkedListNode** head, void* data) {
    // DEBUG TAMBAHAN
    // printf("DEBUG: appendToList dipanggil. *head awal: %p, data: %p\n", (void*)*head, data);
    LinkedListNode* newNode = createLinkedListNode(data);
    if (*head == NULL) { *head = newNode; }
    else {
        LinkedListNode* current = *head;
        while (current->next != NULL) current = current->next;
        current->next = newNode;
    }
    // DEBUG TAMBAHAN
    // printf("DEBUG: appendToList selesai. *head akhir: %p\n", (void*)*head);
}

void removeFromList(LinkedListNode** head, void* data_to_remove) {
    if (!*head || !data_to_remove) return;
    LinkedListNode* current = *head; LinkedListNode* prev = NULL;
    if (current->data == data_to_remove) { *head = current->next; free(current); return; }
    while (current != NULL && current->data != data_to_remove) { prev = current; current = current->next; }
    if (current != NULL) { prev->next = current->next; free(current); }
}

Stack* createStack(int limit) {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    if (!s) { perror("Gagal alokasi Stack"); exit(EXIT_FAILURE); }
    s->top = NULL; s->count = 0; s->limit = limit; return s;
}

int isStackEmpty(Stack* s) { return s == NULL || s->top == NULL; }

int isStackFull(Stack* s) { return s != NULL && s->limit > 0 && s->count >= s->limit; }

void pushUndoAction(Stack* s, UndoAction* action) {
    if (!s || !action) return;
    if (isStackFull(s)) {
        printf("INFO: Undo stack penuh. Aksi terlama tidak dihapus (implementasi sederhana).\n");
        if (s->limit > 0) { free(action); return; }
    }
    LinkedListNode* newNode = createLinkedListNode(action);
    if (!newNode) { free(action); perror("Gagal alokasi Node untuk Stack Undo"); return; }
    newNode->next = s->top; s->top = newNode; s->count++;
}

UndoAction* popUndoAction(Stack* s) {
    if (isStackEmpty(s)) return NULL;
    LinkedListNode* temp = s->top; s->top = s->top->next;
    UndoAction* action = (UndoAction*)temp->data;
    free(temp); s->count--; return action;
}

void freeStackAndActions(Stack* s) {
    if (!s) return;
    while (!isStackEmpty(s)) { UndoAction* action = popUndoAction(s); if (action) free(action); }
    free(s);
}

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) { perror("Gagal alokasi Queue"); exit(EXIT_FAILURE); }
    q->front = q->rear = NULL; q->count = 0; return q;
}

int isQueueEmpty(Queue* q) { return q == NULL || q->front == NULL; }

void enqueueBatchItem(Queue* q, BatchDeleteItem* item) {
    if(!q || !item) return;
    LinkedListNode* newNode = createLinkedListNode(item);
    if (!newNode) { free(item); perror("Gagal alokasi Node untuk Queue Batch"); return;}
    newNode->next = NULL;
    if (q->rear == NULL) q->front = q->rear = newNode;
    else { q->rear->next = newNode; q->rear = newNode; }
    q->count++;
}

BatchDeleteItem* dequeueBatchItem(Queue* q) {
    if (isQueueEmpty(q)) return NULL;
    LinkedListNode* temp = q->front; BatchDeleteItem* item = (BatchDeleteItem*)temp->data;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp); q->count--; return item;
}

void freeQueueAndItems(Queue* q) {
    if (!q) return;
    while (!isQueueEmpty(q)) { BatchDeleteItem* item = dequeueBatchItem(q); if (item) free(item); }
    free(q);
}

void freeQueue(Queue* q) {
    if (q) {
        if (q->front != NULL) {
            // printf("PERINGATAN: freeQueue dipanggil pada queue yang mungkin masih memiliki node (node akan dibebaskan, data diasumsikan sudah bebas).\n");
            LinkedListNode *current = q->front, *next_node;
            while(current != NULL) { next_node = current->next; free(current); current = next_node; }
        }
        free(q);
    }
}

void generateUniqueId(const char* prefix, char* id_buffer) {
    // Menggunakan static counter agar ID unik antar panggilan dalam satu sesi program
    // Untuk keunikan antar sesi program yang berbeda, id_counter perlu di-persist atau
    // menggunakan timestamp yang lebih granular/UUID.
    static long simple_counter_for_id = 0;
    if (id_counter == 0 && simple_counter_for_id == 0) { // Inisialisasi awal jika belum ada dari file
         id_counter = time(NULL) / 100; // Inisialisasi kasar
         if (id_counter == 0) id_counter = 1; // Pastikan tidak 0
    }
    simple_counter_for_id++; // Counter sesi ini
    // Kombinasikan keduanya untuk sedikit variasi, atau pilih salah satu strategi.
    // Untuk debugging, mari gunakan simple_counter_for_id saja agar lebih mudah dilacak.
    // sprintf(id_buffer, "%s_%ld_%ld", prefix, id_counter, simple_counter_for_id);
    sprintf(id_buffer, "%s_%ld", prefix, simple_counter_for_id);
    printf("DEBUG: generateUniqueId: prefix='%s', simple_counter_for_id=%ld, id_buffer Hasil='%s'\n", 
           prefix, simple_counter_for_id, id_buffer);
}

TaskStatus getTaskStatusFromInput() {
    int choice = 0; printf("Pilih Status Tugas:\n");
    for (int i = 0; i < TASK_STATUS_COUNT; i++) printf("%d. %s\n", i + 1, taskStatusToString[i]);
    printf("Pilihan Status (1-%d): ", TASK_STATUS_COUNT);
    if (scanf("%d", &choice) != 1) choice = 0; while(getchar() != '\n'); 
    if (choice > 0 && choice <= TASK_STATUS_COUNT) return (TaskStatus)(choice - 1);
    printf("Pilihan tidak valid, status diatur ke BARU.\n"); return TASK_STATUS_BARU;
}

TaskPriority getTaskPriorityFromInput() {
    int choice = 0; printf("Pilih Prioritas Tugas:\n");
    for (int i = 0; i < TASK_PRIORITY_COUNT; i++) printf("%d. %s\n", i + 1, taskPriorityToString[i]);
    printf("Pilihan Prioritas (1-%d): ", TASK_PRIORITY_COUNT);
    if (scanf("%d", &choice) != 1) choice = 0; while(getchar() != '\n'); 
    if (choice > 0 && choice <= TASK_PRIORITY_COUNT) return (TaskPriority)(choice - 1);
    printf("Pilihan tidak valid, prioritas diatur ke SEDANG.\n"); return TASK_PRIORITY_SEDANG;
}

Project* createProjectInternal(const char* id, const char* name) {
    Project* newProject = (Project*)malloc(sizeof(Project));
    if (!newProject) { perror("Gagal alokasi Project"); return NULL; }

    strncpy(newProject->projectId, id, MAX_ID_LEN - 1); 
    newProject->projectId[MAX_ID_LEN - 1] = '\0';
 
    strncpy(newProject->projectName, name, MAX_NAME_LEN - 1); 
    newProject->projectName[MAX_NAME_LEN - 1] = '\0';
    
    newProject->tasks_head = NULL;
    return newProject;
}

Project* createProject(const char* name) {
    char new_id[MAX_ID_LEN] = {0}; // Inisialisasi buffer
    generateUniqueId("PRJ", new_id);
    
    Project* proj = createProjectInternal(new_id, name);
    if (proj) {
        appendToList(&project_list_head, proj);
        printf("Proyek '%s' (ID: %s) berhasil dibuat.\n", proj->projectName, proj->projectId);
    } else {
        printf("ERROR: Gagal membuat proyek '%s'.\n", name);
    }
    return proj;
}

Task* createTaskInternal(const char* id, const char* name, const char* desc,
                         const char* projectId, const char* parentIdStr,
                         TaskStatus status, TaskPriority priority, const char* dueDate) {
    Task* newTask = (Task*)malloc(sizeof(Task));
    if (!newTask) { perror("Gagal alokasi Task"); return NULL; }

    strncpy(newTask->taskId, id, MAX_ID_LEN -1); newTask->taskId[MAX_ID_LEN-1] = '\0';

    strncpy(newTask->taskName, name, MAX_NAME_LEN -1); newTask->taskName[MAX_NAME_LEN-1] = '\0';
        
    strncpy(newTask->description, desc, MAX_DESC_LEN -1); newTask->description[MAX_DESC_LEN-1] = '\0';
    strncpy(newTask->projectId, projectId, MAX_ID_LEN - 1); newTask->projectId[MAX_ID_LEN - 1] = '\0';
    if (parentIdStr && strcmp(parentIdStr, "NULL") != 0 && strlen(parentIdStr) > 0) {
        strncpy(newTask->parentTaskId, parentIdStr, MAX_ID_LEN - 1); newTask->parentTaskId[MAX_ID_LEN - 1] = '\0';
    } else {
        newTask->parentTaskId[0] = '\0';
    }
    newTask->status = status; newTask->priority = priority;
    if (dueDate && strlen(dueDate) > 0) strncpy(newTask->dueDate, dueDate, DATE_LEN -1); 
    else newTask->dueDate[0] = '\0'; 
    newTask->dueDate[DATE_LEN-1] = '\0';
    newTask->parent = NULL; newTask->children_head = NULL;
    return newTask;
}

Task* createTaskAndRecordUndo(const char* name, const char* desc, Project* project, Task* parentTaskOpt,
                               TaskStatus status, TaskPriority priority, const char* dueDate) {
    if (!project) {
         return NULL;}
    char new_id[MAX_ID_LEN] = {0}; 
    generateUniqueId("TSK", new_id);
    const char* parentIdStr = (parentTaskOpt) ? parentTaskOpt->taskId : "NULL";
    Task* newTask = createTaskInternal(new_id, name, desc, project->projectId, parentIdStr, status, priority, dueDate);
    if (newTask) {
        UndoAction* action = (UndoAction*)malloc(sizeof(UndoAction));
        if (action) {
            action->type = UNDO_TASK_CREATION;
            strncpy(action->taskId, newTask->taskId, MAX_ID_LEN -1); action->taskId[MAX_ID_LEN-1] = '\0';
            strncpy(action->projectId, project->projectId, MAX_ID_LEN-1); action->projectId[MAX_ID_LEN-1] = '\0';
            pushUndoAction(undo_stack, action);
        } else { printf("ERROR: Gagal buat action undo.\n"); free(newTask); return NULL; }
    }
    return newTask;
}

Task* findTaskInProjectRecursive(LinkedListNode* task_node_head, const char* taskId) {
    LinkedListNode* current_node = task_node_head;
    while (current_node != NULL) {
        Task* current_task = (Task*)current_node->data;
        if (strcmp(current_task->taskId, taskId) == 0) return current_task;
        Task* found_in_child = findTaskInProjectRecursive(current_task->children_head, taskId);
        if (found_in_child) return found_in_child;
        current_node = current_node->next;
    }
    return NULL;
}

Task* findTaskInProjectById(Project* project, const char* taskId) {
    if (!project || !taskId) return NULL;
    return findTaskInProjectRecursive(project->tasks_head, taskId);
}

void addChildTask(Task* parentTask, Task* childTask) {
    if (!parentTask || !childTask) return;
    childTask->parent = parentTask; strcpy(childTask->parentTaskId, parentTask->taskId); 
    appendToList(&(parentTask->children_head), childTask);
    printf("Sub-tugas '%s' ditambahkan ke '%s'.\n", childTask->taskName, parentTask->taskName);
}

void addRootTaskToProject(Project* project, Task* task) {
    if (!project || !task) return;
    task->parent = NULL; task->parentTaskId[0] = '\0'; 
    appendToList(&(project->tasks_head), task);
    printf("Tugas utama '%s' ke proyek '%s'.\n", task->taskName, project->projectName);
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

void displayTaskWBS(Task* task, int level) {
    if (!task) return;
    for (int i = 0; i < level; ++i) printf("  "); 
    printf("|- [%s] %s (Status: %s, Prioritas: %s, Due: %s)\n", 
           task->taskId, task->taskName, taskStatusToString[task->status], 
           taskPriorityToString[task->priority], strlen(task->dueDate) > 0 ? task->dueDate : "N/A");
    for (int i = 0; i < level +1; ++i) printf("  ");
    printf("   Desc: %s\n", task->description);
    LinkedListNode* child_node = task->children_head;
    while (child_node != NULL) { displayTaskWBS((Task*)child_node->data, level + 1); child_node = child_node->next; }
}

void displayProjectWBS(Project* project) {
    if (!project) { printf("Proyek tidak ditemukan untuk WBS.\n"); return; }
    printf("\n--- WBS Proyek: %s (ID: %s) ---\n", project->projectName, project->projectId);
    if (project->tasks_head == NULL) printf("Belum ada tugas.\n");
    LinkedListNode* task_node = project->tasks_head;
    while (task_node != NULL) { displayTaskWBS((Task*)task_node->data, 0); task_node = task_node->next; }
    printf("---------------------------------\n");
}

void listAllProjects() {
    printf("\n--- Daftar Proyek ---\n");
    if (project_list_head == NULL) { printf("Belum ada proyek.\n"); printf("---------------------\n"); return; }
    int count = 1; LinkedListNode* current = project_list_head;
    while (current != NULL) {
        Project* p = (Project*)current->data;

        printf("%d. %s (ID: %s)\n", count++, p->projectName, p->projectId);
        current = current->next;
    }
    printf("---------------------\n");
}

void editProjectDetails(Project* project) {
    if (!project) { printf("ERROR: Proyek tidak valid untuk diedit.\n"); return; }
    char name_buffer[MAX_NAME_LEN];
    printf("Nama Proyek saat ini: %s (ID: %s)\n", project->projectName, project->projectId);
    printf("Masukkan Nama Proyek baru (kosongkan jika tidak ingin ubah): ");
    fgets(name_buffer, MAX_NAME_LEN, stdin); name_buffer[strcspn(name_buffer, "\n")] = 0; 
    if (strlen(name_buffer) > 0) {
        strncpy(project->projectName, name_buffer, MAX_NAME_LEN - 1);
        project->projectName[MAX_NAME_LEN - 1] = '\0';
        printf("Nama proyek berhasil diubah menjadi '%s'.\n", project->projectName);
    } else { printf("Nama proyek tidak diubah.\n"); }
}

void editTaskDetails(Task* task) {
    if (!task) { printf("ERROR: Tugas tidak valid untuk diedit.\n"); return; }
    char buffer[MAX_DESC_LEN]; 
    printf("--- Mengedit Tugas: [%s] %s ---\n", task->taskId, task->taskName);
    printf("Nama Tugas saat ini: %s\n", task->taskName);
    printf("Nama Tugas baru (kosongkan jika tidak ubah): ");
    fgets(buffer, MAX_NAME_LEN, stdin); buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) { strncpy(task->taskName, buffer, MAX_NAME_LEN - 1); task->taskName[MAX_NAME_LEN-1] = '\0';}
    printf("Deskripsi saat ini: %s\n", task->description);
    printf("Deskripsi baru (kosongkan jika tidak ubah): ");
    fgets(buffer, MAX_DESC_LEN, stdin); buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) { strncpy(task->description, buffer, MAX_DESC_LEN - 1); task->description[MAX_DESC_LEN-1] = '\0';}
    printf("Status saat ini: %s\n", taskStatusToString[task->status]);
    task->status = getTaskStatusFromInput();
    printf("Prioritas saat ini: %s\n", taskPriorityToString[task->priority]);
    task->priority = getTaskPriorityFromInput();
    printf("Tanggal Tenggat saat ini: %s\n", strlen(task->dueDate) > 0 ? task->dueDate : "N/A");
    printf("Tanggal Tenggat baru (YYYY-MM-DD, kosongkan jika tidak ubah): ");
    fgets(buffer, DATE_LEN, stdin); buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) == 10) { strncpy(task->dueDate, buffer, DATE_LEN - 1); task->dueDate[DATE_LEN-1] = '\0'; }
    else if (strlen(buffer) > 0 && strlen(buffer) != 10) { printf("Format tanggal tidak valid (YYYY-MM-DD). Tidak diubah.\n");}
    else if (strlen(buffer) == 0) { printf("Tanggal tenggat tidak diubah/dikosongkan.\n"); task->dueDate[0] = '\0';} // Opsi mengosongkan
    printf("Detail tugas [%s] diperbarui.\n", task->taskId);
}

void searchProjectsByName(const char* searchTerm) {
    printf("\n--- Hasil Pencarian Proyek untuk '%s' ---\n", searchTerm);
    int found_count = 0; LinkedListNode* current = project_list_head;
    while (current != NULL) {
        Project* p = (Project*)current->data;
        if (strstr(p->projectName, searchTerm) != NULL) { 
            printf("- %s (ID: %s)\n", p->projectName, p->projectId); found_count++;
        }
        current = current->next;
    }
    if (found_count == 0) printf("Tidak ada proyek yang cocok.\n");
    printf("-----------------------------------------\n");
}

void searchTasksInProjectRecursive(LinkedListNode* task_node_head, const char* searchTerm, int* found_count, int level) {
    LinkedListNode* current_node = task_node_head;
    while (current_node != NULL) {
        Task* task = (Task*)current_node->data;
        if (strstr(task->taskName, searchTerm) != NULL || strstr(task->description, searchTerm) != NULL) {
            for (int i = 0; i < level; ++i) printf("  ");
            printf("  |- [%s] %s (Status: %s, Prioritas: %s, Due: %s)\n",
                   task->taskId, task->taskName, taskStatusToString[task->status],
                   taskPriorityToString[task->priority], strlen(task->dueDate) > 0 ? task->dueDate : "N/A");
            (*found_count)++;
        }
        if (task->children_head) searchTasksInProjectRecursive(task->children_head, searchTerm, found_count, level +1);
        current_node = current_node->next;
    }
}
void searchTasksInProjectByName(Project* project, const char* searchTerm) {
    if (!project) { printf("ERROR: Proyek tidak valid untuk pencarian.\n"); return; }
    printf("\n--- Hasil Pencarian Tugas '%s' di Proyek '%s' ---\n", searchTerm, project->projectName);
    int found_count = 0;
    searchTasksInProjectRecursive(project->tasks_head, searchTerm, &found_count, 0);
    if (found_count == 0) printf("Tidak ada tugas yang cocok.\n");
    printf("---------------------------------------------------------\n");
}

void findAndPrintTasksByStatusRecursive(LinkedListNode* task_head, TaskStatus status_to_find, int* count, int level) {
    LinkedListNode* current = task_head;
    while(current) {
        Task* t = (Task*)current->data;
        if (t->status == status_to_find) {
            for (int i = 0; i < level; ++i) printf("  ");
            printf("  |- [%s] %s (Prioritas: %s, Due: %s)\n", t->taskId, t->taskName,
                    taskPriorityToString[t->priority], strlen(t->dueDate) > 0 ? t->dueDate : "N/A");
            (*count)++;
        }
        if (t->children_head) {
            findAndPrintTasksByStatusRecursive(t->children_head, status_to_find, count, level + 1);
        }
        current = current->next;
    }
}
void reportTasksByStatus(Project* project) {
    if (!project) { printf("ERROR: Proyek tidak valid.\n"); return; }
    printf("--- Laporan Tugas berdasarkan Status di Proyek: %s ---\n", project->projectName);
    TaskStatus selected_status = getTaskStatusFromInput();
    printf("--- Tugas Status: %s ---\n", taskStatusToString[selected_status]);
    int found_count = 0;
    findAndPrintTasksByStatusRecursive(project->tasks_head, selected_status, &found_count, 0);
    if (found_count == 0) printf("Tidak ada tugas status '%s'.\n", taskStatusToString[selected_status]);
    printf("-----------------------------------\n");
}
void findAndPrintTasksByPriorityRecursive(LinkedListNode* task_head, TaskPriority prio_to_find, int* count, int level) {
    LinkedListNode* current = task_head;
    while(current) {
        Task* t = (Task*)current->data;
        if (t->priority == prio_to_find) {
            for (int i = 0; i < level; ++i) printf("  ");
            printf("  |- [%s] %s (Status: %s, Due: %s)\n", t->taskId, t->taskName,
                    taskStatusToString[t->status], strlen(t->dueDate) > 0 ? t->dueDate : "N/A");
            (*count)++;
        }
        if (t->children_head) {
            findAndPrintTasksByPriorityRecursive(t->children_head, prio_to_find, count, level + 1);
        }
        current = current->next;
    }
}
void reportTasksByPriority(Project* project) {
    if (!project) { printf("ERROR: Proyek tidak valid.\n"); return; }
    printf("--- Laporan Tugas per Prioritas di Proyek: %s ---\n", project->projectName);
    TaskPriority selected_priority = getTaskPriorityFromInput();
    printf("--- Tugas Prioritas: %s ---\n", taskPriorityToString[selected_priority]);
    int found_count = 0;
    findAndPrintTasksByPriorityRecursive(project->tasks_head, selected_priority, &found_count, 0);
    if (found_count == 0) printf("Tidak ada tugas prioritas '%s'.\n", taskPriorityToString[selected_priority]);
     printf("-----------------------------------\n");
}

void saveTasksOfProject(FILE* file_ptr, LinkedListNode* task_head_node) {
    LinkedListNode* current_task_node = task_head_node;
    while(current_task_node != NULL) {
        Task* task = (Task*)current_task_node->data;
        fprintf(file_ptr, "T,%s,%s,%s,%s,%s,%d,%d,%s\n", 
                task->taskId, task->projectId,
                (task->parent ? task->parent->taskId : "NULL"), 
                task->taskName, task->description,
                (int)task->status, (int)task->priority, task->dueDate[0] == '\0' ? "NULL" : task->dueDate);
        if (task->children_head) saveTasksOfProject(file_ptr, task->children_head);
        current_task_node = current_task_node->next;
    }
}
void saveDataToFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) { printf("ERROR: Gagal buka file '%s' untuk simpan.\n", filename); return; }
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
            appendToList(&(project->tasks_head), task); task->parent = NULL; 
        } else {
            Task* parentTask = NULL; LinkedListNode* search_node = *all_tasks_for_project_head; 
            while(search_node != NULL) {
                Task* potential_parent = (Task*)search_node->data;
                if (task != potential_parent && strcmp(potential_parent->taskId, task->parentTaskId) == 0) {
                    parentTask = potential_parent; break;
                }
                search_node = search_node->next;
            }
            if (parentTask) { task->parent = parentTask; appendToList(&(parentTask->children_head), task); }
            else { printf("WARNING: Parent ID '%s' task '%s' tidak ditemukan.\n", task->parentTaskId, task->taskName);
                   appendToList(&(project->tasks_head), task); task->parent = NULL;}
        }
        current_task_node = current_task_node->next;
    }
}
void loadDataFromFile(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) { printf("INFO: File '%s' tidak ditemukan.\n", filename); return; }
    char line[MAX_LINE_LEN]; Project* current_loading_project = NULL;
    LinkedListNode* temp_task_list_for_current_project = NULL; 
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0; char* type = strtok(line, ",");
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
            char* id = strtok(NULL, ","); char* name = strtok(NULL, "\n"); 
            if (id && name) {
                current_loading_project = createProjectInternal(id, name);
                if (current_loading_project) appendToList(&project_list_head, current_loading_project);
                else printf("ERROR: Gagal load proyek: ID=%s.\n", id);
            }
        } else if (strcmp(type, "T") == 0 && current_loading_project) {
            char* task_id_str = strtok(NULL, ","); char* project_id_str = strtok(NULL, ","); 
            char* parent_task_id_str = strtok(NULL, ","); char* task_name_str = strtok(NULL, ",");
            char* task_desc_str = strtok(NULL, ","); char* status_val_str = strtok(NULL, ",");
            char* priority_val_str = strtok(NULL, ","); char* due_date_val_str = strtok(NULL, "\n"); 
            if (task_id_str && project_id_str && parent_task_id_str && task_name_str && task_desc_str && 
                status_val_str && priority_val_str && due_date_val_str) {
                if (strcmp(project_id_str, current_loading_project->projectId) != 0) {
                    printf("WARNING: Task %s, project ID %s beda dari %s. Dilewati.\n", task_id_str, project_id_str, current_loading_project->projectId);
                    continue;
                }
                TaskStatus status = (TaskStatus)atoi(status_val_str);
                TaskPriority priority = (TaskPriority)atoi(priority_val_str);
                Task* task = createTaskInternal(task_id_str, task_name_str, task_desc_str, project_id_str, 
                                                parent_task_id_str, status, priority, 
                                                (strcmp(due_date_val_str, "NULL") == 0 ? "" : due_date_val_str) );
                if (task) appendToList(&temp_task_list_for_current_project, task);
                else printf("ERROR: Gagal load tugas: ID=%s.\n", task_id_str);
            } else printf("ERROR: Format data tugas tidak lengkap di file untuk baris mulai T,%s...\n", task_id_str ? task_id_str : "(ID tidak terbaca)");
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
    fclose(fp); printf("Data dimuat dari %s.\n", filename);
}

void deepFreeTask(Task* task) {
    if (!task) return;
    LinkedListNode* child_node = task->children_head; LinkedListNode* next_child_node;
    while (child_node != NULL) {
        next_child_node = child_node->next;
        deepFreeTask((Task*)child_node->data); 
        free(child_node); 
        child_node = next_child_node;
    }
    free(task);
}

void deleteTask(Project* project, const char* taskId, int an_undo_action) {
    if (!project || !taskId) { printf("ERROR: Proyek/ID Tugas tidak valid untuk dihapus.\n"); return; }
    Task* task_to_delete = findTaskInProjectById(project, taskId);
    if (!task_to_delete) { printf("ERROR: Tugas ID '%s' tidak ditemukan di proyek '%s'.\n", taskId, project->projectName); return; }
    if (task_to_delete->parent) removeFromList(&(task_to_delete->parent->children_head), task_to_delete);
    else removeFromList(&(project->tasks_head), task_to_delete);
    char taskNameCopy[MAX_NAME_LEN]; strncpy(taskNameCopy, task_to_delete->taskName, MAX_NAME_LEN-1); taskNameCopy[MAX_NAME_LEN-1] = '\0';
    deepFreeTask(task_to_delete); 
    if (!an_undo_action) { 
        printf("Tugas '%s' (ID: %s) dan sub-tugasnya dihapus.\n", taskNameCopy, taskId);
    }
}

void deleteProject(const char* projectId) {
    Project* project_to_delete = findProjectById(projectId);
    if (!project_to_delete) { printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", projectId); return; }
    LinkedListNode* current_task_node = project_to_delete->tasks_head; LinkedListNode* next_task_node;
    while (current_task_node != NULL) {
        next_task_node = current_task_node->next;
        deepFreeTask((Task*)current_task_node->data); 
        free(current_task_node); 
        current_task_node = next_task_node;
    }
    project_to_delete->tasks_head = NULL; 
    removeFromList(&project_list_head, project_to_delete);
    char projectNameCopy[MAX_NAME_LEN]; strncpy(projectNameCopy, project_to_delete->projectName, MAX_NAME_LEN-1); projectNameCopy[MAX_NAME_LEN-1] = '\0';
    free(project_to_delete); 
    printf("Proyek '%s' (ID: %s) dihapus.\n", projectNameCopy, projectId);
}

void freeAllMemory() {
    if (undo_stack) { freeStackAndActions(undo_stack); undo_stack = NULL; }
    if (batch_task_queue) { freeQueueAndItems(batch_task_queue); batch_task_queue = NULL; } 
    LinkedListNode* current_project_node = project_list_head; LinkedListNode* next_project_node;
    while (current_project_node != NULL) {
        next_project_node = current_project_node->next;
        Project* project = (Project*)current_project_node->data;
        LinkedListNode* current_task_node = project->tasks_head; LinkedListNode* next_task_node;
        while (current_task_node != NULL) {
            next_task_node = current_task_node->next;
            deepFreeTask((Task*)current_task_node->data); 
            free(current_task_node); 
            current_task_node = next_task_node;
        }
        free(project); 
        free(current_project_node); 
        current_project_node = next_project_node;
    }
    project_list_head = NULL; 
}

void processUndoLastTaskCreation() {
    if (isStackEmpty(undo_stack)) { printf("INFO: Tidak ada aksi pembuatan tugas untuk di-undo.\n"); return; }
    UndoAction* last_action = popUndoAction(undo_stack);
    if (last_action && last_action->type == UNDO_TASK_CREATION) {
        Project* project = findProjectById(last_action->projectId);
        if (project) {
            printf("UNDO: Menghapus tugas dengan ID '%s' dari proyek '%s'.\n", last_action->taskId, project->projectName);
            deleteTask(project, last_action->taskId, 1);
        } else { printf("ERROR: Proyek ID '%s' untuk undo tidak ditemukan.\n", last_action->projectId); }
        free(last_action);
    } else if (last_action) {
        printf("ERROR: Aksi undo terakhir bukan pembuatan tugas.\n"); free(last_action);
    }
}

void processBatchDeleteTasks(Project* project) {
    if (!project) { printf("ERROR: Proyek tidak valid untuk batch delete.\n"); return; }
    if (batch_task_queue) { freeQueueAndItems(batch_task_queue); batch_task_queue = NULL;}
    batch_task_queue = createQueue();
    char task_id_input[MAX_ID_LEN];
    printf("Masukkan ID tugas yang akan dihapus secara batch (ketik 'done' untuk selesai):\n");
    displayProjectWBS(project);
    while(1) {
        printf("ID Tugas (atau 'done'): ");
        fgets(task_id_input, MAX_ID_LEN, stdin);
        task_id_input[strcspn(task_id_input, "\n")] = 0;
        if (strcmp(task_id_input, "done") == 0) break;
        Task* task_to_check = findTaskInProjectById(project, task_id_input);
        if (task_to_check) {
            BatchDeleteItem* item = (BatchDeleteItem*)malloc(sizeof(BatchDeleteItem));
            if(item) {
                strncpy(item->taskId, task_to_check->taskId, MAX_ID_LEN-1); item->taskId[MAX_ID_LEN-1] = '\0';
                enqueueBatchItem(batch_task_queue, item);
                printf("Tugas ID '%s' ditambahkan ke antrian batch hapus.\n", task_to_check->taskId);
            } else { printf("ERROR: Gagal alokasi memori untuk item batch.\n");}
        } else { printf("WARNING: Tugas ID '%s' tidak ditemukan dalam proyek ini.\n", task_id_input); }
    }
    if (isQueueEmpty(batch_task_queue)) { printf("Tidak ada tugas dalam antrian untuk dihapus.\n"); }
    else {
        printf("\nMemproses batch hapus...\n"); int count = 0;
        while(!isQueueEmpty(batch_task_queue)) {
            BatchDeleteItem* item_to_delete = dequeueBatchItem(batch_task_queue);
            if (item_to_delete) { deleteTask(project, item_to_delete->taskId, 0); free(item_to_delete); count++; }
        }
        printf("%d tugas diproses dari antrian batch hapus.\n", count);
    }
    if (batch_task_queue) { freeQueue(batch_task_queue); batch_task_queue = NULL; }
}

void displayMenu() {
    printf("\n=== Program Manajemen Proyek (v0.9.1 - Debugging) ===\n");
    printf("1. Buat Proyek Baru\n");
    printf("2. Edit Detail Proyek\n");
    printf("3. Manajemen & Pencarian Proyek/Tugas\n");
    printf("4. Tambah Tugas ke Proyek\n");
    printf("5. Edit Detail Tugas\n");
    printf("6. Lihat WBS Proyek\n");
    printf("7. Operasi Hapus\n");
    printf("8. Manajemen Data (Simpan/Muat)\n");
    printf("9. Undo Tambah Tugas Terakhir\n");
    printf("10. Laporan Tugas\n");
    printf("0. Keluar\n");
    printf("Pilihan Anda: ");
}

int getSubMenuChoice(int max_option) {
    int sub_choice = -1;
    if (scanf("%d", &sub_choice) != 1) {
        printf("Input sub-menu tidak valid.\n"); while (getchar() != '\n'); return -1;
    }
    while (getchar() != '\n');
    if (sub_choice < 0 || sub_choice > max_option) { // Perbolehkan 0 untuk kembali
        printf("Pilihan sub-menu tidak valid.\n"); return -1;
    }
    return sub_choice;
}

int main() {
    srand(time(NULL)); 
    id_counter = 0; // Biarkan generateUniqueId yang pertama kali menginisialisasi dengan time

    undo_stack = createStack(MAX_UNDO_ACTIONS);
    loadDataFromFile(DATA_FILE); 

    int choice = -1, sub_choice = -1;
    char name_buffer[MAX_NAME_LEN], desc_buffer[MAX_DESC_LEN], due_date_buffer[DATE_LEN];
    char id_buffer[MAX_ID_LEN], parent_id_buffer[MAX_ID_LEN], project_id_buffer[MAX_ID_LEN]; 
    Project* current_project = NULL; Task* current_task = NULL; Task* parent_task = NULL;
    TaskStatus status_input; TaskPriority priority_input;

    do {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            printf("ERROR: Input tidak valid.\n"); while (getchar() != '\n'); 
            choice = -100; continue;
        }
        while (getchar() != '\n'); 

        switch (choice) {
            case 1:
                printf("Nama Proyek Baru: "); 
                fgets(name_buffer, MAX_NAME_LEN, stdin); 
                name_buffer[strcspn(name_buffer, "\n")] = 0; 
                createProject(name_buffer);
                if (project_list_head && project_list_head->data) {
                     Project* p_debug = (Project*)project_list_head->data; // Asumsi proyek baru di head
                     // Jika appendToList menambahkan ke akhir, ini tidak akan selalu proyek baru.
                     // Untuk debug yang lebih baik, createProject sebaiknya return Project* dan kita cek itu.
                     // Atau kita cari proyek terakhir di list.
                     // Mari kita coba lihat yang terakhir jika ada
                     LinkedListNode* temp_node = project_list_head;
                     Project* last_added_proj = NULL;
                     if(temp_node){
                        while(temp_node->next != NULL) temp_node = temp_node->next;
                        last_added_proj = (Project*)temp_node->data;
                     }
                }
                break;
            case 2: 
                listAllProjects();
                printf("ID Proyek diedit: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                current_project = findProjectById(project_id_buffer);
                if (current_project) editProjectDetails(current_project);
                else printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer);
                break;
            case 3: 
                printf("\n--- Manajemen & Pencarian Proyek/Tugas ---\n");
                printf("1. Lihat Semua Proyek\n"); printf("2. Cari Proyek berdasarkan Nama\n");
                printf("3. Cari Tugas dalam Proyek (Nama/Desk)\n"); printf("0. Kembali ke Menu Utama\n");
                printf("Pilihan Sub-Menu: "); sub_choice = getSubMenuChoice(3);
                switch (sub_choice) {
                    case 1: listAllProjects(); break;
                    case 2:
                        printf("Masukkan nama proyek dicari: "); fgets(name_buffer, MAX_NAME_LEN, stdin); name_buffer[strcspn(name_buffer, "\n")] = 0;
                        searchProjectsByName(name_buffer); break;
                    case 3:
                        listAllProjects();
                        printf("ID Proyek untuk cari tugas: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                        current_project = findProjectById(project_id_buffer);
                        if (current_project) {
                            printf("Nama/kata kunci tugas dicari: "); fgets(name_buffer, MAX_NAME_LEN, stdin); name_buffer[strcspn(name_buffer, "\n")] = 0;
                            searchTasksInProjectByName(current_project, name_buffer);
                        } else printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer);
                        break;
                    case 0: printf("Kembali ke menu utama...\n"); break;
                }
                break;
            case 4: 
                listAllProjects();
                printf("ID Proyek untuk tambah tugas: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                current_project = findProjectById(project_id_buffer);
                if (!current_project) { printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer); break; }
                printf("Nama Tugas: "); fgets(name_buffer, MAX_NAME_LEN, stdin); name_buffer[strcspn(name_buffer, "\n")] = 0;
                printf("Deskripsi Tugas: "); fgets(desc_buffer, MAX_DESC_LEN, stdin); desc_buffer[strcspn(desc_buffer, "\n")] = 0;
                status_input = getTaskStatusFromInput(); priority_input = getTaskPriorityFromInput();
                printf("Tanggal Tenggat (YYYY-MM-DD, kosongkan jika tidak ada): ");
                fgets(due_date_buffer, DATE_LEN, stdin); due_date_buffer[strcspn(due_date_buffer, "\n")] = 0;
                printf("Sub-tugas? (y/n): "); char is_subtask_char; scanf("%c", &is_subtask_char); while (getchar() != '\n'); 
                if (is_subtask_char == 'y' || is_subtask_char == 'Y') {
                    displayProjectWBS(current_project); 
                    printf("ID Tugas Induk: "); fgets(parent_id_buffer, MAX_ID_LEN, stdin); parent_id_buffer[strcspn(parent_id_buffer, "\n")] = 0;
                    parent_task = findTaskInProjectById(current_project, parent_id_buffer);
                    if (!parent_task) printf("ERROR: Tugas Induk ID '%s' tidak ditemukan.\n", parent_id_buffer);
                    else {
                        current_task = createTaskAndRecordUndo(name_buffer, desc_buffer, current_project, parent_task, status_input, priority_input, due_date_buffer);
                        if (current_task) addChildTask(parent_task, current_task); else printf("ERROR: Gagal buat tugas.\n");
                    }
                } else {
                    current_task = createTaskAndRecordUndo(name_buffer, desc_buffer, current_project, NULL, status_input, priority_input, due_date_buffer);
                    if (current_task) addRootTaskToProject(current_project, current_task); else printf("ERROR: Gagal buat tugas.\n");
                }
                break;
            case 5: 
                listAllProjects();
                printf("ID Proyek dari tugas diedit: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                current_project = findProjectById(project_id_buffer);
                if (current_project) {
                    displayProjectWBS(current_project);
                    printf("ID Tugas diedit: "); fgets(id_buffer, MAX_ID_LEN, stdin); id_buffer[strcspn(id_buffer, "\n")] = 0;
                    current_task = findTaskInProjectById(current_project, id_buffer);
                    if (current_task) editTaskDetails(current_task);
                    else printf("ERROR: Tugas ID '%s' tidak ditemukan.\n", id_buffer);
                } else printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer);
                break;
            case 6: 
                listAllProjects();
                printf("ID Proyek untuk lihat WBS: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                current_project = findProjectById(project_id_buffer);
                if (current_project) displayProjectWBS(current_project);
                else printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer);
                break;
            case 7: 
                printf("\n--- Operasi Hapus ---\n");
                printf("1. Hapus Tugas (Tunggal)\n"); printf("2. Hapus Proyek\n");
                printf("3. Batch Hapus Tugas Dalam Proyek\n"); printf("0. Kembali ke Menu Utama\n");
                printf("Pilihan Sub-Menu: "); sub_choice = getSubMenuChoice(3);
                switch (sub_choice) {
                    case 1: 
                        listAllProjects();
                        printf("ID Proyek tugas: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                        current_project = findProjectById(project_id_buffer);
                        if (!current_project) { printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer); break; }
                        displayProjectWBS(current_project);
                        printf("ID Tugas dihapus: "); fgets(id_buffer, MAX_ID_LEN, stdin); id_buffer[strcspn(id_buffer, "\n")] = 0;
                        deleteTask(current_project, id_buffer, 0); break;
                    case 2: 
                        listAllProjects();
                        printf("ID Proyek dihapus: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                        deleteProject(project_id_buffer); break;
                    case 3: 
                        listAllProjects();
                        printf("ID Proyek untuk batch hapus: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                        current_project = findProjectById(project_id_buffer);
                        if (current_project) processBatchDeleteTasks(current_project);
                        else printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer);
                        break;
                    case 0: printf("Kembali ke menu utama...\n"); break;
                }
                break;
            case 8: 
                printf("\n--- Manajemen Data ---\n");
                printf("1. Simpan Data ke File\n"); printf("2. Muat Data dari File\n");
                printf("0. Kembali ke Menu Utama\n"); printf("Pilihan Sub-Menu: ");
                sub_choice = getSubMenuChoice(2);
                switch (sub_choice) {
                    case 1: saveDataToFile(DATA_FILE); break;
                    case 2:
                        printf("Peringatan: Muat data akan menimpa data saat ini (termasuk histori undo).\n");
                        printf("Lanjutkan? (y/n): "); char confirm_load; scanf("%c", &confirm_load); while (getchar() != '\n');
                        if (confirm_load == 'y' || confirm_load == 'Y') {
                            freeAllMemory(); 
                            undo_stack = createStack(MAX_UNDO_ACTIONS); 
                            project_list_head = NULL; 
                            loadDataFromFile(DATA_FILE);
                        } else printf("INFO: Pemuatan data dibatalkan.\n");
                        break;
                    case 0: printf("Kembali ke menu utama...\n"); break;
                }
                break;
            case 9: processUndoLastTaskCreation(); break;
            case 10: 
                listAllProjects();
                printf("ID Proyek untuk laporan: "); fgets(project_id_buffer, MAX_ID_LEN, stdin); project_id_buffer[strcspn(project_id_buffer, "\n")] = 0;
                current_project = findProjectById(project_id_buffer);
                if (current_project) {
                    printf("\n--- Laporan Tugas untuk Proyek: %s ---\n", current_project->projectName);
                    printf("1. Laporan berdasarkan Status\n"); printf("2. Laporan berdasarkan Prioritas\n");
                    printf("0. Kembali ke Menu Utama\n"); printf("Pilihan Sub-Menu Laporan: ");
                    sub_choice = getSubMenuChoice(2);
                    switch (sub_choice) {
                        case 1: reportTasksByStatus(current_project); break;
                        case 2: reportTasksByPriority(current_project); break;
                        case 0: printf("Kembali ke menu utama...\n"); break;
                    }
                } else printf("ERROR: Proyek ID '%s' tidak ditemukan.\n", project_id_buffer);
                break;
            case 0: 
                printf("INFO: Simpan data sebelum keluar...\n"); saveDataToFile(DATA_FILE); 
                printf("Keluar dari program...\n");
                break;
            default: printf("ERROR: Pilihan tidak valid.\n");
        }
    } while (choice != 0);

    freeAllMemory();
    printf("Semua memori dibebaskan. Program selesai.\n");
    return 0;
}