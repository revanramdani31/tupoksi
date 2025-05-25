// -----------------------------------------------------------------------------
// project_manager.c – Sistem Manajemen Proyek (ADT Lengkap)
// Dosen: Struktur Data & Algoritma
// Fitur  :
//   • Tree ADT (CRUD, delete rekursif, save, load)
//   • Priority‑Queue ADT (array‑sorted ascending priority)
//   • Stack ADT (undo pekerjaan)
//   • Linked‑List ADT (riwayat tugas selesai + timestamp)
//   • File I/O (tab‑indented save/load)
// -----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================
// KONSTANTA GLOBAL
// ============================================================
#define MAX_NAME   50
#define MAX_DESC   120
#define DATE_LEN   16      /* yyyy-mm-dd\0 */
#define MAX_CHILD  10
#define MAX_TUGAS  200
#define MAX_DEPTH  100     /* kedalaman tree pada saat load */

// ============================================================
// ENUM STATUS
// ============================================================
typedef enum {TODO, IN_PROGRESS, DONE} Status;

const char* statusStr(Status s){
    switch(s){
        case TODO:        return "Belum dimulai";
        case IN_PROGRESS: return "Sedang dikerjakan";
        case DONE:        return "Selesai";
    }
    return "?";
}

// ============================================================
// TREE ADT  (struktur proyek)
// ============================================================
typedef struct TreeNode{
    char               name[MAX_NAME];
    char               desc[MAX_DESC];
    int                est_hours;                 // estimasi jam
    char               deadline[DATE_LEN];        // yyyy-mm-dd
    Status             status;
    struct TreeNode*   child[MAX_CHILD];
    int                child_count;
} TreeNode;

TreeNode* node_create(const char* n,const char* d,int est,const char* dl){
    TreeNode* t=(TreeNode*)calloc(1,sizeof(TreeNode));
    strncpy(t->name,n,MAX_NAME);
    strncpy(t->desc,d,MAX_DESC);
    t->est_hours=est;
    strncpy(t->deadline,dl,DATE_LEN);
    t->status=TODO;
    return t;
}

void node_add_child(TreeNode* p,TreeNode* c){ if(p&&c&&p->child_count<MAX_CHILD) p->child[p->child_count++]=c; }

TreeNode* node_find(TreeNode* r,const char* target){
    if(!r) return NULL;
    if(strcmp(r->name,target)==0) return r;
    for(int i=0;i<r->child_count;i++){
        TreeNode* res=node_find(r->child[i],target);
        if(res) return res;
    }
    return NULL;
}

void node_print(TreeNode* r,int lvl){
    if(!r) return;
    for(int i=0;i<lvl;i++) printf("  ");
    printf("- %s [%s] (ETC %dh, DL %s)\n",r->name,statusStr(r->status),r->est_hours,r->deadline);
    for(int i=0;i<r->child_count;i++) node_print(r->child[i],lvl+1);
}

void node_count(TreeNode* r,int* tot,int* done){
    if(!r) return;
    (*tot)++;
    if(r->status==DONE) (*done)++;
    for(int i=0;i<r->child_count;i++) node_count(r->child[i],tot,done);
}

void node_save(FILE* f,TreeNode* r,int lvl){
    if(!r) return;
    for(int i=0;i<lvl;i++) fputc('\t',f);
    fprintf(f,"%s|%s|%d|%s|%d\n",r->name,r->desc,r->est_hours,r->deadline,r->status);
    for(int i=0;i<r->child_count;i++) node_save(f,r->child[i],lvl+1);
}

TreeNode* node_load(FILE* f){
    char line[512];
    TreeNode* levelNode[MAX_DEPTH];
    int       levelIdx[MAX_DEPTH];
    int top=-1; TreeNode* root=NULL;

    while(fgets(line,sizeof(line),f)){
        int lvl=0; while(line[lvl]=='\t') lvl++;
        if(lvl>=MAX_DEPTH) continue;
        char* data=line+lvl;

        char name[MAX_NAME],desc[MAX_DESC],ddl[DATE_LEN]; int est,st;
        if(sscanf(data,"%49[^|]|%119[^|]|%d|%15[^|]|%d",name,desc,&est,ddl,&st)!=5) continue;

        TreeNode* n=node_create(name,desc,est,ddl); n->status=(Status)st;

        if(lvl==0){ root=n; top=0; levelNode[top]=n; levelIdx[top]=0; }
        else{
            while(top>=0 && levelIdx[top]>=lvl) top--; if(top<0) {free(n); continue;}
            node_add_child(levelNode[top],n);
            top++; levelNode[top]=n; levelIdx[top]=lvl;
        }
    }
    return root;
}

void free_tree(TreeNode* r){ if(!r) return; for(int i=0;i<r->child_count;i++) free_tree(r->child[i]); free(r);} 

int node_delete(TreeNode* parent,const char* target){
    if(!parent) return 0;
    for(int i=0;i<parent->child_count;i++){
        if(strcmp(parent->child[i]->name,target)==0){
            TreeNode* del=parent->child[i];
            for(int j=i+1;j<parent->child_count;j++) parent->child[j-1]=parent->child[j];
            parent->child_count--; free_tree(del); return 1;
        }
        if(node_delete(parent->child[i],target)) return 1;
    }
    return 0;
}

void node_edit(TreeNode* n){
    if(!n) return; char buf[128];
    printf("=== Edit Tugas: %s ===\n",n->name);
    printf("Nama          [%s]: "); fgets(buf,MAX_NAME,stdin); if(buf[0]!='\n'){buf[strcspn(buf,"\n")]=0; strncpy(n->name,buf,MAX_NAME);} 
    printf("Deskripsi     [%s]: "); fgets(buf,MAX_DESC,stdin); if(buf[0]!='\n'){buf[strcspn(buf,"\n")]=0; strncpy(n->desc,buf,MAX_DESC);} 
    printf("Estimasi jam  [%d] : "); fgets(buf,MAX_DESC,stdin); if(buf[0]!='\n') n->est_hours=atoi(buf);
    printf("Deadline      [%s] : "); fgets(buf,DATE_LEN,stdin); if(buf[0]!='\n'){buf[strcspn(buf,"\n")]=0; strncpy(n->deadline,buf,DATE_LEN);} 
    puts("Status (0=TODO 1=IN_PROGRESS 2=DONE)");
    printf("Status        [%s] : "); fgets(buf,MAX_DESC,stdin); if(buf[0]!='\n'){int st=atoi(buf); if(st>=0&&st<=2) n->status=(Status)st;}
    puts("Tugas diperbarui!");
}

// ============================================================
// PRIORITY‑QUEUE ADT
// ============================================================
typedef struct{ char tugas[MAX_TUGAS][MAX_NAME]; int prio[MAX_TUGAS]; int count;} PQueue;
void pq_init(PQueue* q){ q->count=0; }
int  pq_empty(PQueue* q){ return q->count==0; }
void pq_enqueue(PQueue* q,const char* t,int p){
    if(q->count>=MAX_TUGAS){ puts("Antrian penuh!"); return; }
    int i=q->count-1; while(i>=0 && p<q->prio[i]){ strcpy(q->tugas[i+1],q->tugas[i]); q->prio[i+1]=q->prio[i]; i--; }
    strcpy(q->tugas[i+1],t); q->prio[i+1]=p; q->count++; }
char* pq_dequeue(PQueue* q){ if(pq_empty(q)){ puts("Antrian kosong!"); return NULL;} static char ret[MAX_NAME]; strcpy(ret,q->tugas[0]); for(int i=1;i<q->count;i++){ strcpy(q->tugas[i-1],q->tugas[i]); q->prio[i-1]=q->prio[i]; } q->count--; return ret; }
void pq_print(PQueue* q){ puts("\n-- Antrian Tugas (Prioritas Rendah → Tinggi) --"); for(int i=0;i<q->count;i++) printf("%d. %s (P=%d)\n",i+1,q->tugas[i],q->prio[i]); }

// ============================================================
// STACK ADT (undo)
// ============================================================
typedef struct{ char data[MAX_TUGAS][MAX_NAME]; int top; } Stack;
void stack_init(Stack* s){ s->top=-1; }
int  stack_empty(Stack* s){ return s->top==-1; }
void stack_push(Stack* s,const char* t){ if(s->top<MAX_TUGAS-1) strcpy(s->data[++s->top],t); }
char* stack_pop(Stack* s){ return stack_empty(s)?NULL:s->data[s->top--]; }

// ============================================================
// LINKED‑LIST ADT (riwayat selesai)
// ============================================================
typedef struct LNode{ char tugas[MAX_NAME]; time_t waktu; struct LNode* next;} LNode;
void list_push(LNode** h,const char* t){ LNode* n=(LNode*)malloc(sizeof(LNode)); strncpy(n->tugas,t,MAX_NAME); n->waktu=time(NULL); n->next=*h; *h=n; }
void list_print(LNode* h){ int idx=1; char buf[32]; while(h){ strftime(buf,32,"%Y-%m-%d %H:%M",localtime(&h->waktu)); printf("%d) %s [%s]\n",idx++,h->tugas,buf); h=h->next;} }
void list_free(LNode* h){ while(h){ LNode* tmp=h; h=h->next; free(tmp);} }

// ============================================================
// UTILITIES
// ============================================================
void flushstdin(void){ int c; while((c=getchar())!='\n' && c!=EOF); }
void getline_trim(char* buf,int len){ fgets(buf,len,stdin); buf[strcspn(buf,"\n")]=0; }

// ============================================================
// MENU
// ============================================================
void menu(){
    puts("\n==== SISTEM MANAJEMEN PROYEK ====");
    puts("0. Muat proyek dari file");
    puts("1. Tampilkan struktur proyek");
    puts("2. Tambah tugas/subtugas");
    puts("3. Edit detail tugas");
    puts("4. Hapus tugas/subtugas");
    puts("5. Perbarui status tugas");
    puts("6. Tambah tugas ke antrian");
    puts("7. Kerjakan tugas (dequeue)");
    puts("8. Undo kerjaan terakhir");
    puts("9. Laporan progres");
    puts("10. Simpan proyek ke file");
    puts("11. Keluar");
    printf("Pilihan: ");
}

// ============================================================
// PROGRAM UTAMA
// ============================================================
int main(){
    char buf[MAX_DESC];
    printf("Nama proyek root: "); getline_trim(buf,MAX_NAME);
    if(strlen(buf)==0) strcpy(buf,"Proyek Utama");
    TreeNode* root=node_create(buf,"Proyek Utama",0,"-");

    PQueue pq; pq_init(&pq);
    Stack undo; stack_init(&undo);
    LNode* selesai=NULL;

    int opt;
    while(1){
        menu(); if(scanf("%d",&opt)!=1){ flushstdin(); continue;} flushstdin();
        switch(opt){
            case 0:{
                printf("Nama file: "); getline_trim(buf,MAX_NAME);
                FILE* f=fopen(buf,"r"); if(!f){ puts("Gagal buka file!"); break; }
                TreeNode* tmp=node_load(f); fclose(f);
                if(tmp){ free_tree(root); root=tmp; puts("Proyek dimuat!"); }
            }break;
            case 1: node_print(root,0); break;
            case 2:{
                char parent[MAX_NAME], nm[MAX_NAME], desc[MAX_DESC], dl[DATE_LEN]; int est;
                printf("Induk (kosong=root): "); getline_trim(parent,MAX_NAME);
                TreeNode* p=(strlen(parent)==0)?root:node_find(root,parent);
                if(!p){ puts("Induk tidak ditemukan!"); break; }
                printf("Nama tugas : "); getline_trim(nm,MAX_NAME);
                printf("Deskripsi  : "); getline_trim(desc,MAX_DESC);
                printf("Estimasi jam: "); scanf("%d",&est); flushstdin();
                printf("Deadline (yyyy-mm-dd): "); getline_trim(dl,DATE_LEN);
                node_add_child(p,node_create(nm,desc,est,dl)); puts("Tugas ditambahkan!");
            }break;
            case 3:{ printf("Nama tugas: "); getline_trim(buf,MAX_NAME); TreeNode* n=node_find(root,buf); if(!n){ puts("Tidak ditemukan!"); break;} node_edit(n);} break;
            case 4:{ printf("Nama tugas dihapus: "); getline_trim(buf,MAX_NAME); if(node_delete(root,buf)) puts("Tugas terhapus."); else puts("Tidak ditemukan!"); } break;
            case 5:{ printf("Nama tugas: "); getline_trim(buf,MAX_NAME); TreeNode* n=node_find(root,buf); if(!n){ puts("Tidak ditemukan!"); break;} puts("Status (0=TODO 1=IN_PROGRESS 2=DONE)"); int st; scanf("%d",&st); flushstdin(); if(st>=0&&st<=2){ n->status=(Status)st; if(st==DONE) list_push(&selesai,n->name);} } break;
            case 6:{ printf("Nama tugas: "); getline_trim(buf,MAX_NAME); int p; printf("Prioritas (1=tinggi..9=rendah): "); scanf("%d",&p); flushstdin(); pq_enqueue(&pq,buf,p); } break;
            case 7:{ char* t=pq_dequeue(&pq); if(t) { printf("Mengerjakan: %s\n",t); stack_push(&undo,t);} } break;
            case 8:{ char* last=stack_pop(&undo); if(last){ printf("Undo: %s\n",last); pq_enqueue(&pq,last,1);} else puts("Tidak ada yang bisa diundo!"); } break;
            case 9:{ int tot=0,done=0; node_count(root,&tot,&done); printf("Total tugas: %d, Selesai: %d (%.2f%%)\n",tot,done,tot?done*100.0/tot:0); puts("Riwayat selesai:"); list_print(selesai); pq_print(&pq);} break;
            case 10:{ printf("Nama file: "); getline_trim(buf,MAX_NAME); FILE* f=fopen(buf,"w"); if(!f){ puts("Gagal simpan!"); break;} node_save(f,root,0); fclose(f); puts("Proyek tersimpan!"); } break;
            case 11: free_tree(root); list_free(selesai); puts("Keluar..."); return 0;
            default: puts("Pilihan salah!");
        }
    }
    return 0;
}
