#ifndef MENU_H
#define MENU_H

#include "utils.h"
#include "project.h"
#include "task.h"

// Function prototypes
void displayMainMenu();
void displayProjectMenu();
void displayTaskMenu();
void displayLogMenu();
void displayBatchMenu();
void displayFileMenu();

void handleProjectMenu();
void handleTaskMenu();
void handleLogMenu();
void handleBatchMenu();
void handleFileMenu();
void handleUndo();
void runMainMenu();

#endif // MENU_H 
