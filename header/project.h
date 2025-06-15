#ifndef PROJECT_H
#define PROJECT_H

#include "task.h"

#define MAX_NAME_LEN 100
#define MAX_DESC_LEN 500
#define MAX_ID_LEN 20
#define DATE_LEN 11

typedef struct Project {
    char projectId[MAX_ID_LEN];
    char projectName[MAX_NAME_LEN];
    Task* rootTasks;  
} Project;

extern Project** projects;
extern int projectCount;
extern int projectCapacity;

typedef struct {
    char changeId[MAX_ID_LEN];
    char description[MAX_DESC_LEN];
    char timestamp[DATE_LEN];
    char userId[MAX_ID_LEN];
    char changeType[MAX_NAME_LEN];  
} ChangeLog;

Project* createProjectInternal(const char* id, const char* name);
Project* createProject(const char* name);
Project* findProjectById(const char* projectId);
void editProjectDetails(Project* project);
void deleteProject(const char* projectId);
void listAllProjects();

void displayProjectWBS(Project* project);
void displayUpcomingTasks(Project* project);

void recordChange(const char* description, const char* userId, const char* changeType);
void displayChangeLog();
void searchChangeLog(const char* searchTerm);
void exportChangeLogToCSV();

void initProjectArray();
void expandProjectArray();
void addProject(Project* project);
void removeProject(Project* project);
void freeProjectArray();

void recordChange(const char* description, const char* userId, const char* changeType);
void displayChangeLog();
void searchChangeLog(const char* searchTerm);
void exportChangeLog(const char* filename);

#endif 
