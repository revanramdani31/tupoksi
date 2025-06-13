#ifndef MENU_H
#define MENU_H

#include "utils.h"
#include "project.h"
#include "task.h"

// Function prototypes
void displayMainMenu();
void displayProjectMenu();
void displayTaskMenu();
void displayReportMenu();
void displayLogMenu();
void handleProjectMenu();
void handleTaskMenu();
void handleReportMenu();
void handleLogMenu();
void handleUndo();
void runMainMenu();

#endif // MENU_H 