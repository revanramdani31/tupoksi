#ifndef SRC_UTILS_H
#define SRC_UTILS_H

#include "task.h"
#include "utils.h"

// Utility functions
void clearBuffer();
char* getInput(const char* prompt, char* buffer, int maxLen);
int validateDate(const char* date);
void formatDate(char* dest, const char* src);

void searchTasksByName(Task* root, const char* searchTerm, int* foundCount, int level);
void findAndPrintTasksByStatus(Task* root, TaskStatus status, int* foundCount, int level);
int getConfirmation(const char* message);

#endif // SRC_UTILS_H 