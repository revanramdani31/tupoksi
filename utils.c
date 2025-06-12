#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "task.h"

// Global variables
long id_counter = 0;

// String arrays for task status and priority
const char* taskStatusToString[] = {"Baru", "Dikerjakan", "Selesai", "Tertunda"};
const char* taskPriorityToString[] = {"Rendah", "Sedang", "Tinggi", "Mendesak"};

char* generateUniqueId(const char* prefix) {
    static long simple_counter_for_id = 0;
    static char id_buffer[MAX_ID_LEN];
    
    if (id_counter == 0 && simple_counter_for_id == 0) {
        id_counter = (long)time(NULL);
    }
    
    simple_counter_for_id++;
    snprintf(id_buffer, MAX_ID_LEN, "%s%ld", prefix, simple_counter_for_id);
    return id_buffer;
}

int getSubMenuChoice(int max_option) {
    int choice;
    printf("Pilihan: ");
    scanf("%d", &choice);
    getchar(); // Consume newline
    
    if (choice >= 0 && choice <= max_option) {
        return choice;
    }
    
    printf("Pilihan tidak valid.\n");
    return -1;
} 