#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "task.h" 

#define MAX_NAME_LEN 100
#define MAX_DESC_LEN 500
#define DATE_LEN 11
#define MAX_LINE_LEN 1024
#define MAX_UNDO_ACTIONS 100
#define DATA_DIR "data/"
#define DATA_FILE DATA_DIR "project_data.txt"
#define LOG_FILE DATA_DIR "change_log.csv"
#define MAX_ID_LEN 20

typedef struct Task Task;
typedef struct Project Project;

char* generateUniqueId(const char* prefix);
int getSubMenuChoice(int max_option);
int getConfirmation(const char* message);

void clearBuffer();
void searchTasksByName(Task* root, const char* searchTerm, int* foundCount, int level);
void findAndPrintTasksByStatus(Task* root, TaskStatus status, int* foundCount, int level);
char* getInput(const char* prompt, char* buffer, int maxLen);
char* generateProjectId();
char* generateTaskId();
char* generateChangeId();
int validateDate(const char* date);
void formatDate(char* dest, const char* src);

#endif 
