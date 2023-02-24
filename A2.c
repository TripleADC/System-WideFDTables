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
#include <math.h>
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
    struct FDInfo_struct *FDlist;

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
    new_node->FDlist = NULL;
    new_node->next = NULL;

    return new_node;
}

FDNode *createFDNode(int FD, char filename[STR_LEN])
{
    FDNode *new_node = (FDNode *)calloc(1, sizeof(FDNode));

    if(new_node == NULL) return NULL;

    new_node->FD = FD;
    strcpy(new_node->filename, filename);
    new_node->next = NULL;

    return new_node;
}

processNode *insertProcessList(processNode *root, int PID)
{
    // Inserting depending on the PID
    processNode *traverser = root;
    processNode *new_node = createProcessNode(PID);

    // If root is NULL, then the new_node is the start of the list
    if((root == NULL)) 
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

FDNode *insertFDList(FDNode *root, int FD, char filename[STR_LEN])
{
    // Inserting depending on the PID
    FDNode *traverser = root;
    FDNode *new_node = createFDNode(FD, filename);

    // If root is NULL, then the new_node is the start of the list
    if((root == NULL)) 
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
        printf("FD: %d ", traverser->FD);
        printf("%s\n", traverser->filename);

        traverser = traverser->next;
    }
}

/*
    MAIN FUNCTIONS
*/
FDNode *getFD(int PID)
{   
    // PID list
    FDNode *root = NULL;

    char PIDstr[25];
    char filename[STR_LEN];
    char path[STR_LEN] = "/proc/";

    snprintf(PIDstr, sizeof(PIDstr), "%d", PID);

    // Making path into the format: /proc/[PID]/fd/
    strcat(path, PIDstr);
    strcat(path, "/fd/");
    printf("%s\n", path);
    
    // Opening and traversing proc file
    DIR *procDir = opendir(path);

    if(procDir != NULL)
    {
        struct dirent *currEntry = readdir(procDir);

        while(currEntry != NULL)
        {
            // Updating path
            if(isNumber(currEntry->d_name) == 1 && access(path, R_OK) == 0)
            {
                // Updating paths, filenames, etc.
                strcat(path, currEntry->d_name);
                readlink(path, filename, STR_LEN);

                // Inserting into list
                root = insertFDList(root, atoi(currEntry->d_name), filename);

                // Resetting strings for next entry
                strncpy(filename, "", sizeof(filename));
                strcpy(path, "/proc/");
                strcat(path, PIDstr);
                strcat(path, "/fd/");
            } 
            
            currEntry = readdir(procDir);
        }
    }

    closedir(procDir);
    return root;
}

processNode *getPID()
{
    // PID list
    processNode *root = NULL;

    // Opening and traversing proc file
    DIR *procDir = opendir("/proc");
    struct dirent *currEntry = readdir(procDir);
    struct stat currInfo;

    char path[STR_LEN] = "/proc/";

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
            }

            strncpy(path, "/proc/", sizeof(path));
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
    processNode *traverser = root->next;

    char path[STR_LEN] = "/proc/";
    char PIDstr[25];

    // TO-DO: can be moved into its own function populateFD()
    while(traverser != NULL)
    {
        snprintf(PIDstr, sizeof(PIDstr), "%d", traverser->PID);
        strcat(path, PIDstr);

        if(access(path, R_OK) == 0)
        {
            traverser->FDlist = getFD(traverser->PID);
            printFDList(traverser->FDlist);
        }

        printf("PID: %d\n", traverser->PID);
       
        // Resetting path
        strncpy(path, "/proc/", sizeof(path));
        
        traverser = traverser->next;
    }
    
    return 0;
}
