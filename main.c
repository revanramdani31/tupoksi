#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TUGAS 100
#define MAX_CHILD 10
#define MAX_NAME 50

// ---------------------- STRUCTURE TREE ----------------------
typedef struct TreeNode {
    char nama[MAX_NAME];
    struct TreeNode* child[MAX_CHILD];
    int child_count;
} TreeNode;

TreeNode* buatNode(char* nama) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    strcpy(node->nama, nama);
    node->child_count = 0;
    return node;
}

void tambahChild(TreeNode* parent, TreeNode* child) {
    if (parent->child_count < MAX_CHILD) {
        parent->child[parent->child_count++] = child;
    }
}

void tampilkanTree(TreeNode* root, int level) {
    for (int i = 0; i < level; i++) printf("  ");
    printf("- %s\n", root->nama);
    for (int i = 0; i < root->child_count; i++) {
        tampilkanTree(root->child[i], level + 1);
    }
}

// ---------------------- STRUCTURE QUEUE ----------------------
typedef struct {
    char tugas[MAX_TUGAS][MAX_NAME];
    int front, rear;
} Queue;

void initQueue(Queue* q) {
    q->front = q->rear = -1;
}

int isEmptyQueue(Queue* q) {
    return q->front == -1;
}

int isFullQueue(Queue* q) {
    return q->rear == MAX_TUGAS - 1;
}

void enqueue(Queue* q, char* tugas) {
    if (isFullQueue(q)) {
        printf("Queue penuh!\n");
        return;
    }
    if (isEmptyQueue(q)) q->front = 0;
    strcpy(q->tugas[++q->rear], tugas);
    printf("Tugas '%s' ditambahkan ke antrian.\n", tugas);
}

char* dequeue(Queue* q) {
    if (isEmptyQueue(q)) {
        printf("Queue kosong!\n");
        return NULL;
    }
    char* tugas = q->tugas[q->front];
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front++;
    }
    return tugas;
}

// ---------------------- STRUCTURE STACK ----------------------
typedef struct {
    char data[MAX_TUGAS][MAX_NAME];
    int top;
} Stack;

void initStack(Stack* s) {
    s->top = -1;
}

int isEmptyStack(Stack* s) {
    return s->top == -1;
}

int isFullStack(Stack* s) {
    return s->top == MAX_TUGAS - 1;
}

void push(Stack* s, char* tugas) {
    if (isFullStack(s)) {
        printf("Stack penuh!\n");
        return;
    }
    strcpy(s->data[++s->top], tugas);
    printf("Tugas '%s' disimpan ke riwayat.\n", tugas);
}

char* pop(Stack* s) {
    if (isEmptyStack(s)) {
        printf("Stack kosong!\n");
        return NULL;
    }
    return s->data[s->top--];
}

// ---------------------- MAIN PROGRAM ----------------------
int main() {
    TreeNode* proyek;
    char namaProyek[MAX_NAME];

    printf("Masukkan nama proyek umum Anda (misal: Event, Tesis, Kantor, dll): ");
    fgets(namaProyek, MAX_NAME, stdin);
    namaProyek[strcspn(namaProyek, "\n")] = 0; // hapus newline
    proyek = buatNode(namaProyek);

    Queue antrian;
    Stack riwayat;
    initQueue(&antrian);
    initStack(&riwayat);

    int pilihan;
    char input[MAX_NAME];
    TreeNode* nodeBaru;

    while (1) {
        printf("\n=== Sistem Manajemen Proyek Umum ===\n");
        printf("1. Tampilkan Struktur Proyek\n");
        printf("2. Tambah Subtugas (Node Baru)\n");
        printf("3. Tambah Tugas ke Antrian\n");
        printf("4. Ambil Tugas dari Antrian (Kerjakan)\n");
        printf("5. Undo Tugas Terakhir\n");
        printf("6. Keluar\n");
        printf("Pilih menu: ");
        scanf("%d", &pilihan);
        getchar(); // membersihkan buffer

        switch (pilihan) {
            case 1:
                printf("\nStruktur Proyek:\n");
                tampilkanTree(proyek, 0);
                break;
            case 2:
                printf("Masukkan nama tugas induk (kosongkan jika root): ");
                fgets(input, MAX_NAME, stdin);
                input[strcspn(input, "\n")] = 0;
                TreeNode* parentNode = proyek;

                if (strlen(input) > 0) {
                    // pencarian sederhana (DFS satu level)
                    for (int i = 0; i < proyek->child_count; i++) {
                        if (strcmp(proyek->child[i]->nama, input) == 0) {
                            parentNode = proyek->child[i];
                            break;
                        }
                    }
                }

                printf("Masukkan nama subtugas: ");
                fgets(input, MAX_NAME, stdin);
                input[strcspn(input, "\n")] = 0;
                nodeBaru = buatNode(input);
                tambahChild(parentNode, nodeBaru);
                printf("Subtugas '%s' ditambahkan ke '%s'.\n", input, parentNode->nama);
                break;
            case 3:
                printf("Masukkan nama tugas: ");
                fgets(input, MAX_NAME, stdin);
                input[strcspn(input, "\n")] = 0;
                enqueue(&antrian, input);
                break;
            case 4:
                if (!isEmptyQueue(&antrian)) {
                    char* tugas = dequeue(&antrian);
                    printf("Mengambil tugas: %s\n", tugas);
                    push(&riwayat, tugas);
                }
                break;
            case 5:
                if (!isEmptyStack(&riwayat)) {
                    char* batal = pop(&riwayat);
                    printf("Undo tugas: %s\n", batal);
                    enqueue(&antrian, batal);
                }
                break;
            case 6:
                printf("Keluar dari program.\n");
                exit(0);
            default:
                printf("Pilihan tidak valid!\n");
        }
    }
    return 0;
}