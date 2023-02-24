/*
    Assignment 2 : Recreating the System-Wide FD Tables

    Ashtian Dela Cruz
    ashtian.delacruz@mail.utoronto.ca
    1008154710
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define STR_LEN 1024

// Need a CDT
typedef struct FDInfo_struct
{
    int FD;
    char filename[STR_LEN];

    // Will be a linked list of FD's
    struct FDInfo_struct *next; 
} FDNode;

typedef struct processInfo_struct
{
    int PID;

    // Will contain a linked list of FD's that the process has
    struct FDInfo_struct *FD_list;

    // Will be a linked list of PID's
    struct processInfo_struct *next;
} processNode;

/*
    HELPER FUNCTIONS
*/ 
int isNumber(char *numStr)
{
    // Checks if a string is a number by checking if each character in the string is a digit
    for(int i = 0; i < strlen(numStr); i++)
    {
        if(isdigit(numStr[i]) == 0)
        {
            return 0;
        }
    }
    return 1;
}

/*
    LIST FUNCTIONS
*/

processNode *createProcessNode(int PID)
{
    processNode *new_node = (processNode *)calloc(1, sizeof(processNode));

    if(new_node == NULL) return NULL;

    new_node->PID = PID;
    new_node->FD_list = NULL;
    new_node->next = NULL;

    return new_node;
}

processNode *createFDNode(int FD, char name[STR_LEN])
{
    processNode *new_node = (FDNode *)calloc(1, sizeof(FDNode));

    if(new_node == NULL) return NULL;

    new_node->FD = FD;
    strcpy(new_node->name, name);
    new_node->next = NULL;

    return new_node;
}

processNode *insertProcessList(processNode *root, int PID)
{
    // Inserting depending on the PID
    processNode *traverser = root;
    processNode *new_node = createProcessNode(PID);

    // If root is NULL, then the new_node is the start of the list
    if((root == NULL) || (PID < root->PID)) 
    {
        return new_node;
    }

    // Finding proper place in list
    while((traverser->next != NULL))
    {
        traverser = traverser->next;
    }

    // Inserting
    traverser->next = new_node;
    
    return root;
}

FDNode *insertFDList(FDNode *root, int FD, char name[STR_LEN])
{
    // Inserting depending on the PID
    FDNode *traverser = root;
    FDNode *new_node = createFDNode(FD, name);

    // If root is NULL, then the new_node is the start of the list
    if((root == NULL) || (FD < root->FD)) 
    {
        return new_node;
    }

    // Finding proper place in list
    while((traverser->next != NULL))
    {
        traverser = traverser->next;
    }

    // Inserting
    traverser->next = new_node;
    
    return root;
}

processNode *deleteProcessList(processNode *root)
{
    processNode *traverser = root;
    processNode *q = NULL;

    while(traverser->next != NULL)
    {
        q = traverser->next;
        free(traverser);
        traverser = q;
    }

    return NULL;
}

FDNode *deleteFDList(FDNode *root)
{
    FDNode *traverser = root;
    FDNode *q = NULL;

    while(traverser->next != NULL)
    {
        q = traverser->next;
        free(traverser);
        traverser = q;
    }

    return NULL;
}

processNode *printProcessList(processNode *root)
{
    processNode *traverser = root;

    while(traverser != NULL)
    {
        printf("%d\n", traverser->PID);

        traverser = traverser->next;
    }
}

FDNode *printFDList(FDNode *root)
{
    FDNode *traverser = root;

    while(traverser != NULL)
    {
        printf("%d\n", traverser->FD);

        traverser = traverser->next;
    }
}

/*
    MAIN FUNCTIONS
*/
void getFD(char *path)
{   
    char path2[100] = "";
    strcpy(path2, path);
    strcat(path2, "/");
    printf("%s\n", path2);

    char filename[STR_LEN];

    // Opening and traversing proc file
    DIR *procDir = opendir(path);
    struct dirent *currEntry = readdir(procDir);
    struct stat currInfo;

    while(currEntry != NULL)
    {
        // Updating path
        if(isNumber(currEntry->d_name) == 1)
        {
            // Updating path
            strcat(path2, currEntry->d_name);
            lstat(path2, &currInfo);

            readlink(path2, filename, STR_LEN);

            printf("FD: %s", currEntry->d_name);
            printf("| Inode: %ld", currInfo.st_ino);
            printf("| Filename: %s\n", filename);

            strncpy(filename, "", sizeof(filename));
            strcpy(path2, path);
            strcat(path2, "/");
        } 
        
        currEntry = readdir(procDir);
    }

    closedir(procDir);
}

processNode *getPID()
{

    // PID list
    processNode *root = NULL;

    // Opening and traversing proc file
    DIR *procDir = opendir("/proc");
    struct dirent *currEntry = readdir(procDir);
    struct stat currInfo;

    char path[100] = "/proc/";

    // Since we only want processes owned by the current user
    char *username = getlogin();
    struct passwd *user = getpwnam(username);
    printf("%s %d\n", user->pw_name, user->pw_uid);

    while(currEntry != NULL)
    {
        // Checking if the directory name is a number
        // If it is, it is a PID
        if(isNumber(currEntry->d_name) == 1)
        {
            // Updating path
            strcat(path, currEntry->d_name);
            lstat(path, &currInfo);
            
            // Only want if the user owns the process
            if(currInfo.st_uid == user->pw_uid)
            {
                printf("%s\n", currEntry->d_name);
                root = insertProcessList(root, atoi(currEntry->d_name));

                strcat(path, "/fd");
                
                // if(access(path, R_OK) == 0)
                // {
                //     getFD(path);
                // }
            }

            strcpy(path, "/proc/");
        }
        
        currEntry = readdir(procDir);
    }

    closedir(procDir);
    return root;
}

// Per-process table => Prints PID, FD

// System-wide table => Prints PID, FD, Filename

// VNode table => Prints inodes

// Composite/default => Prints PID, FD, Filename, Inode

// Prints to screen depending on flag
void printToScreen(processNode *root)
{
    printProcessList(root);
}

// Processes flags
int main()
{
    processNode *root = getPID();

    printf("List:\n");
    printToScreen(root);

    return 0;
}
