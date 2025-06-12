#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constants
#define MAX_ID_LEN 32
#define MAX_NAME_LEN 100
#define MAX_DESC_LEN 500
#define DATE_LEN 11
#define MAX_LINE_LEN 1024
#define MAX_UNDO_ACTIONS 100
#define DATA_FILE "project_data.txt"

// Forward declarations
typedef struct LinkedListNode LinkedListNode;
typedef struct Task Task;
typedef struct Project Project;
typedef struct Stack Stack;
typedef struct Queue Queue;
typedef struct UndoAction UndoAction;
typedef struct BatchDeleteItem BatchDeleteItem;

// Function prototypes
char* generateUniqueId(const char* prefix);
int getSubMenuChoice(int max_option);

#endif // UTILS_H 