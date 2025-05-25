#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
//  UNIVERSAL CONSTANTS & TYPES
// ============================================================
#define MAX_NAME   50
#define MAX_DESC   120
#define DATE_LEN   16     /* yyyy-mm-dd\0 */
#define MAX_CHILD  10
#define MAX_TUGAS  200

typedef enum {TODO, IN_PROGRESS, DONE} Status;

const char* statusStr(Status s){
    switch(s){
        case TODO: return "Belum dimulai";
        case IN_PROGRESS: return "Sedang dikerjakan";
        case DONE: return "Selesai";
    }
    return "?";
}

// ============================================================
//  TREE ADT : STRUKTUR PROYEK (CRUD)
// ============================================================
typedef struct TreeNode{
    char               name[MAX_NAME];
    char               desc[MAX_DESC];
    int                est_hours;
    char               deadline[DATE_LEN];
    Status             status;
    struct TreeNode*   child[MAX_CHILD];
    int                child_count;
}TreeNode;

TreeNode* node_create(const char* name,const char* desc,int est,const char* ddl){
    TreeNode* n = (TreeNode*)malloc(sizeof(TreeNode));
    strncpy(n->name,name,MAX_NAME);
    strncpy(n->desc,desc,MAX_DESC);
    n->est_hours = est;
    strncpy(n->deadline,ddl,DATE_LEN);
    n->status = TODO;
    n->child_count = 0;
    return n;
}

void node_add_child(TreeNode* parent,TreeNode* child){
    if(parent->child_count<MAX_CHILD){
        parent->child[parent->child_count++]=child;
    }
}

TreeNode* node_find(TreeNode* root,const char* target){
    if(strcmp(root->name,target)==0) return root;
    for(int i=0;i<root->child_count;i++){
        TreeNode* res = node_find(root->child[i],target);
        if(res) return res;
    }
    return NULL;
}

void node_print(TreeNode* root,int lvl){
    for(int i=0;i<lvl;i++) printf("  ");
    printf("- %s [%s] (ETC %dh, DL %s)\n",root->name,statusStr(root->status),root->est_hours,root->deadline);
    for(int i=0;i<root->child_count;i++) node_print(root->child[i],lvl+1);
}

void node_count(TreeNode* root,int* total,int* done){
    (*total)++;
    if(root->status==DONE) (*done)++;
    for(int i=0;i<root->child_count;i++) node_count(root->child[i],total,done);
}

void node_save(FILE* f,TreeNode* root,int lvl){
    for(int i=0;i<lvl;i++) fputc('\t',f);
    fprintf(f,"%s|%s|%d|%s|%d\n",root->name,root->desc,root->est_hours,root->deadline,root->status);
    for(int i=0;i<root->child_count;i++) node_save(f,root->child[i],lvl+1);
}

void node_edit(TreeNode* n){
    char buf[MAX_DESC];
    printf("=== Edit Tugas: %s ===\n",n->name);
    printf("Nama          [%s] : ",n->name); fgets(buf,MAX_NAME,stdin); if(buf[0]!='\n') {buf[strcspn(buf,"\n")]=0; strncpy(n->name,buf,MAX_NAME);} 
    printf("Deskripsi     [%s] : ",n->desc); fgets(buf,MAX_DESC,stdin); if(buf[0]!='\n'){buf[strcspn(buf,"\n")]=0; strncpy(n->desc,buf,MAX_DESC);} 
    printf("Estimasi jam  [%d]  : ",n->est_hours); fgets(buf,MAX_DESC,stdin); if(buf[0]!='\n'){n->est_hours=atoi(buf);} 
    printf("Deadline      [%s] : ",n->deadline); fgets(buf,DATE_LEN,stdin); if(buf[0]!='\n'){buf[strcspn(buf,"\n")]=0; strncpy(n->deadline,buf,DATE_LEN);} 
    puts("Status: 0=TODO 1=IN_PROGRESS 2=DONE");
    printf("Status        [%s] : ",statusStr(n->status)); fgets(buf,MAX_DESC,stdin); if(buf[0]!='\n'){int st=atoi(buf); if(st>=0&&st<=2) n->status=(Status)st;}
    puts("Tugas diperbarui!");
}

// ============================================================
//  LINKED LIST : RIWAYAT TUGAS SELESAI
// ============================================================
typedef struct LNode{
    char tugas[MAX_NAME];
    struct LNode* next;
}LNode;

void list_push(LNode** head,const char* t){
    LNode* n=(LNode*)malloc(sizeof(LNode));
    strncpy(n->tugas,t,MAX_NAME);
    n->next=*head;
    *head=n;
}

void list_print(LNode* head){
    int idx=1;
    while(head){
        printf("%d. %s\n",idx++,head->tugas);
        head=head->next;
    }
}

// ============================================================
//  STACK ADT :  UNDO ACTIONS
// ============================================================
typedef struct{
    char data[MAX_TUGAS][MAX_NAME];
    int top;
}Stack;

void stack_init(Stack* s){s->top=-1;}
int stack_empty(Stack* s){return s->top==-1;}
void stack_push(Stack* s,const char* t){ if(s->top<MAX_TUGAS-1) strncpy(s->data[++s->top],t,MAX_NAME);} 
char* stack_pop(Stack* s){ return stack_empty(s)?NULL:s->data[s->top--]; }

// ============================================================
//  PRIORITY QUEUE : ARRAY IMPLEMENTATION
// ============================================================
typedef struct{
    char tugas[MAX_TUGAS][MAX_NAME];
    int  prio[MAX_TUGAS];      // lebih kecil = lebih prioritas
    int  count;
}PQueue;

void pq_init(PQueue* q){q->count=0;}
int pq_empty(PQueue* q){return q->count==0;}

void pq_enqueue(PQueue* q,const char* tugas,int priority){
    if(q->count==MAX_TUGAS){puts("Antrian penuh!");return;}
    int i=q->count-1;
    while(i>=0 && priority<q->prio[i]){
        strcpy(q->tugas[i+1],q->tugas[i]);
        q->prio[i+1]=q->prio[i];
        i--; }
    strcpy(q->tugas[i+1],tugas);
    q->prio[i+1]=priority;
    q->count++; }

char* pq_dequeue(PQueue* q){
    if(pq_empty(q)){puts("Antrian kosong!");return NULL;}
    char* t=q->tugas[0];
    for(int i=1;i<q->count;i++){
        strcpy(q->tugas[i-1],q->tugas[i]);
        q->prio[i-1]=q->prio[i]; }
    q->count--; return t; }

void pq_print(PQueue* q){
    puts("\n-- Antrian Tugas (Prioritas Rendah -> Tinggi) --");
    for(int i=0;i<q->count;i++) printf("%d. %s (P=%d)\n",i+1,q->tugas[i],q->prio[i]); }

// ============================================================
//  UTILITAS I/O
// ============================================================
void flushStdin(void){int c;while((c=getchar())!='\n'&&c!=EOF);} 

void getLine(char* buf,int len){
    fgets(buf,len,stdin);
    buf[strcspn(buf,"\n")]=0;
}

// ============================================================
//  MENU OPERASI
// ============================================================
void tampilkanMenu(){
    puts("\n=== SISTEM MANAJEMEN PROYEK (CRUD) ===");
    puts("1. Tampilkan Struktur Proyek (Read)");
    puts("2. Tambah Tugas/Subtugas (Create)");
    puts("3. Edit Detail Tugas (Update)");
    puts("4. Hapus Tugas/Subtugas (Delete)");
    puts("5. Perbarui Status Tugas (Update)");
    puts("6. Tambah Tugas ke Antrian Prioritas");
    puts("7. Kerjakan Tugas (Dequeue)");
    puts("8. Undo Kerjaan Terakhir");
    puts("9. Laporan Progres");
    puts("10. Simpan Proyek ke File");
    puts("11. Keluar");
    printf("Pilihan: ");
}

// ============================================================
//  MAIN PROGRAM
// ============================================================
int main(){
    char buf[MAX_DESC];
    printf("Masukkan nama proyek root: ");
    getLine(buf,MAX_NAME);
    TreeNode* root=node_create(buf,"Proyek Utama",0,"-");

    PQueue queue; pq_init(&queue);
    Stack undo; stack_init(&undo);
    LNode* doneList=NULL;

    int choice;
    while(1){
        tampilkanMenu();
        if(scanf("%d",&choice)!=1){flushStdin();continue;} flushStdin();
        switch(choice){
            case 1: node_print(root,0); break;
            case 2:{
                char parentName[MAX_NAME];
                printf("Nama tugas induk (kosong = root): "); getLine(parentName,MAX_NAME);
                TreeNode* parent = (strlen(parentName)==0)? root : node_find(root,parentName);
                if(!parent){puts("Induk tidak ditemukan!");break;}
                char name[MAX_NAME], desc[MAX_DESC], ddl[DATE_LEN]; int est;
                printf("Nama subtugas: "); getLine(name,MAX_NAME);
                printf("Deskripsi: "); getLine(desc,MAX_DESC);
                printf("Estimasi jam: "); scanf("%d",&est); flushStdin();
                printf("Deadline (yyyy-mm-dd): "); getLine(ddl,DATE_LEN);
                TreeNode* n=node_create(name,desc,est,ddl);
                node_add_child(parent,n);
                printf("Subtugas '%s' ditambahkan.\n",name);
            }break;
            case 3:{
                printf("Nama tugas yang akan diedit: "); getLine(buf,MAX_NAME);
                TreeNode* t=node_find(root,buf);
                if(!t){puts("Tidak ditemukan!");break;}
                node_edit(t);
            }break;
            case 4:{
                printf("Nama tugas yang dihapus: "); getLine(buf,MAX_NAME);
                TreeNode* parent=NULL, *target=NULL;
                TreeNode* qarr[256]; int qs=0,qe=0;
                qarr[qe++]=root;
                while(qs<qe && !target){
                    TreeNode* cur=qarr[qs++];
                    for(int i=0;i<cur->child_count;i++){
                        if(strcmp(cur->child[i]->name,buf)==0){parent=cur;target=cur->child[i];break;}
                        qarr[qe++]=cur->child[i];
                    }
                }
                if(!target){puts("Tidak ditemukan!");break;}
                int idx=0; while(idx<parent->child_count && parent->child[idx]!=target) idx++;
                for(int i=idx+1;i<parent->child_count;i++) parent->child[i-1]=parent->child[i];
                parent->child_count--; free(target);
                puts("Tugas dihapus.");
            }break;
            case 5:{
                printf("Nama tugas: "); getLine(buf,MAX_NAME);
                TreeNode* t=node_find(root,buf);
                if(!t){puts("Tidak ditemukan!");break;}
                puts("Status: 0=TODO 1=IN_PROGRESS 2=DONE");
                int st; scanf("%d",&st); flushStdin();
                if(st>=0&&st<=2){
                    t->status=(Status)st;
                    if(st==DONE) list_push(&doneList,t->name);
                }
            }break;
            case 6:{
                char tname[MAX_NAME]; int pr;
                printf("Nama tugas: "); getLine(tname,MAX_NAME);
                printf("Prioritas (1=tinggi 5=rendah): "); scanf("%d",&pr); flushStdin();
                pq_enqueue(&queue,tname,pr);
            }break;
            case 7:{
                if(pq_empty(&queue)){puts("Antrian kosong!");break;}
                char* t=pq_dequeue(&queue);
                printf("Mengerjakan: %s\n",t);
                stack_push(&undo,t);
            }break;
            case 8:{
                if(stack_empty(&undo)){puts("Tidak ada yang bisa di-undo");break;}
                char* last=stack_pop(&undo);
                printf("Undo tugas: %s (dimasukkan kembali ke antrian)\n",last);
                pq_enqueue(&queue,last,1);
            }break;
            case 9:{
                int total=0,done=0; node_count(root,&total,&done);
                printf("Total tugas: %d, Selesai: %d (%.2f%%)\n",total,done,(total? (done*100.0/total):0));
                puts("Daftar selesai:"); list_print(doneList);
                pq_print(&queue);
            }break;
            case 10:{
                printf("Nama file: "); getLine(buf,MAX_NAME);
                FILE* f=fopen(buf,"w"); if(!f){puts("Gagal buka file!");break;}
                node_save(f,root,0); fclose(f); puts("Struktur disimpan."); }
                break;
            case 11: puts("Keluar..."); exit(0);
            default: puts("Pilihan salah!");
        }
    }
    return 0;
}