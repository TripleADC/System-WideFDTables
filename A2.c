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
    long inode;

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

FDNode *createFDNode(int FD, char filename[STR_LEN], int inode)
{
    FDNode *new_node = (FDNode *)calloc(1, sizeof(FDNode));

    if(new_node == NULL) return NULL;

    new_node->FD = FD;
    strcpy(new_node->filename, filename);
    new_node->inode = inode;
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

FDNode *insertFDList(FDNode *root, int FD, char filename[STR_LEN], long inode)
{
    // Inserting depending on the PID
    FDNode *traverser = root;
    FDNode *new_node = createFDNode(FD, filename, inode);

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

FDNode *deleteFDList(FDNode *root)
{
    FDNode *traverser = root;
    FDNode *q = NULL;

    while(traverser != NULL)
    {
        q = traverser->next;
        free(traverser);
        traverser = q;
    }

    return NULL;
}

processNode *deleteProcessList(processNode *root)
{
    processNode *traverser = root;
    processNode *q = NULL;

    while(traverser != NULL)
    {
        deleteFDList(traverser->FDlist);
        q = traverser->next;
        free(traverser);
        traverser = q;
    }

    return NULL;
}

// processNode *printProcessList(processNode *root)
// {
//     processNode *traverser = root;

//     while(traverser != NULL)
//     {
//         printf("%d\n", traverser->PID);

//         traverser = traverser->next;
//     }
// }

// FDNode *printFDList(FDNode *root)
// {
//     FDNode *traverser = root;

//     while(traverser != NULL)
//     {
//         printf("FD: %d ", traverser->FD);
//         printf("%s\n", traverser->filename);

//         traverser = traverser->next;
//     }
// }

/*
    MAIN FUNCTIONS
*/
int getInodeNumber(int PID, int FD)
{
    char PIDstr[25];
    char FDstr[25];
    char filename[STR_LEN];
    char path[STR_LEN] = "/proc/";

    char inodeStr[25] = "";
    char traverserStr[STR_LEN] = "";
    char *substr;
    
    snprintf(PIDstr, sizeof(PIDstr), "%d", PID);
    snprintf(FDstr, sizeof(FDstr), "%d", FD);

    // Making path into the format: /proc/[PID]/fdinfo/
    strcat(path, PIDstr);
    strcat(path, "/fdinfo/");
    strcat(path, FDstr);

    FILE *fdinfo_file = fopen(path, "r");

    if(fdinfo_file != NULL)
    {
        while(fscanf(fdinfo_file, "%s", traverserStr) != EOF)
        {
            substr = strstr(traverserStr, "ino:");
            if(substr != NULL)
            {
                fscanf(fdinfo_file, "%s", inodeStr);
                break;
            }
        }
    }

    // if(strcmp(inodeStr, "") == 0)
    // {
    //     printf("PID: %d, FD: %d, has no inode in fdinfo\n", PID, FD);
    // }

    fclose(fdinfo_file);
    return strtol(inodeStr, NULL, 10);
}

FDNode *getFD(int PID)
{   
    // PID list
    FDNode *root = NULL;

    char PIDstr[25];
    char filename[STR_LEN];
    char path[STR_LEN] = "/proc/";
    int inode = -1;

    snprintf(PIDstr, sizeof(PIDstr), "%d", PID);

    // Making path into the format: /proc/[PID]/fd/
    strcat(path, PIDstr);
    strcat(path, "/fd/");
    
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
                inode = getInodeNumber(PID, atoi(currEntry->d_name));

                // Inserting into list
                root = insertFDList(root, atoi(currEntry->d_name), filename, inode);

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
                root = insertProcessList(root, atoi(currEntry->d_name));
            }

            strncpy(path, "/proc/", sizeof(path));
        }
        
        currEntry = readdir(procDir);
    }

    closedir(procDir);
    return root;
}

void populateFD(processNode *root)
{
    processNode *traverser = root;
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
        }
       
        // Resetting path
        strncpy(path, "/proc/", sizeof(path));
        
        traverser = traverser->next;
    }
}

// Per-process table => Prints PID, FD
void printPerProcess(processNode *root, int hasPositional, int argPID)
{
    processNode *processTraverser = root;
    FDNode *FDTraverser = NULL;
    int i = 0;

    printf("\t PID \t\t FD\n");
    printf("=============================================\n");
    while(processTraverser != NULL)
    {
        FDTraverser = processTraverser->FDlist;

        if((hasPositional == 0) || (hasPositional && argPID == processTraverser->PID))
        {
            if(FDTraverser == NULL)
            {
                printf("%d \t %d \t N/A\n", i, processTraverser->PID);
                i++;
            }
            else
            {
                while(FDTraverser != NULL)
                {
                    printf("%d \t %d \t %d\n", i, processTraverser->PID, FDTraverser->FD);
                    FDTraverser = FDTraverser->next;
                    i++;
                }
            }
        }
        
        processTraverser = processTraverser->next;
        i++;
    }
}

// System-wide table => Prints PID, FD, Filename
void printSystemWide(processNode *root, int hasPositional, int argPID)
{
    processNode *processTraverser = root;
    FDNode *FDTraverser = NULL;
    int i = 0;

    printf("\t PID \t FD \t File name\n");
    printf("=============================================\n");
    while(processTraverser != NULL)
    {
        FDTraverser = processTraverser->FDlist;

        if((hasPositional == 0) || (hasPositional && argPID == processTraverser->PID))
        {
            if(FDTraverser == NULL)
            {
                printf("%d \t %d  N/A \t N/A\n", i, processTraverser->PID);
                i++;
            }
            else
            {
                while(FDTraverser != NULL)
                {
                    printf("%d \t %d  %d \t %s\n", i, processTraverser->PID, FDTraverser->FD, FDTraverser->filename);
                    FDTraverser = FDTraverser->next;
                    i++;
                }
            }
        }
        
        processTraverser = processTraverser->next;
        i++;
    }
}

// VNode table => Prints inodes
void printVNodes(processNode *root, int hasPositional, int argPID)
{
    processNode *processTraverser = root;
    FDNode *FDTraverser = NULL;
    int i = 0;

    printf("\t FD \t\t\t Inode\n");
    printf("=============================================\n");
    while(processTraverser != NULL)
    {
        FDTraverser = processTraverser->FDlist;

        if((hasPositional == 0) || (hasPositional && argPID == processTraverser->PID))
        {
            if(FDTraverser == NULL)
            {
                printf("%d \t %d \t N/A\n", i, processTraverser->PID);
                i++;
            }
            else
            {
                while(FDTraverser != NULL)
                {
                    printf("%d \t %d \t %ld\n", i, FDTraverser->FD, FDTraverser->inode);
                    FDTraverser = FDTraverser->next;
                    i++;
                }
            }
        }
        
        processTraverser = processTraverser->next;
        i++;
    }
}

// Composite/default => Prints PID, FD, Filename, Inode
void printComposite(processNode *root, int hasPositional, int argPID)
{
    processNode *processTraverser = root;
    FDNode *FDTraverser = NULL;
    int i = 0;

    printf("\t PID \t FD \t File name \t\t Inode \n");
    printf("=============================================================================================\n");
    while(processTraverser != NULL)
    {
        FDTraverser = processTraverser->FDlist;

        if((hasPositional == 0) || (hasPositional && argPID == processTraverser->PID))
        {
            if(FDTraverser == NULL)
            {
                printf("%d \t %d  N/A \t N/A\n", i, processTraverser->PID);
                i++;
            }
            else
            {
                while(FDTraverser != NULL)
                {
                    printf("%d \t %d  %d \t %s \t %ld\n", i, processTraverser->PID, FDTraverser->FD, FDTraverser->filename, FDTraverser->inode);
                    FDTraverser = FDTraverser->next;
                    i++;
                }
            }
        }
        
        processTraverser = processTraverser->next;
        i++;
    }
}

void printThresholdList(processNode *root, int thresholdNum)
{
    processNode *processTraverser = root;
    FDNode *FDTraverser = NULL;

    while(processTraverser != NULL)
    {
        FDTraverser = processTraverser->FDlist;
        while(FDTraverser != NULL)
        {
            if(FDTraverser->FD > thresholdNum)
            {
                printf("%d (%d),\n", processTraverser->PID, FDTraverser->FD);
            }
            
            FDTraverser = FDTraverser->next;
        }

        processTraverser = processTraverser->next;
    }
}

// Prints to screen depending on flag
void printToScreen(processNode *root, int canPrintPerProcess, int canPrintSystemWide, int canPrintVNodes, int canPrintComposite, int hasThreshold, int thresholdNum, int hasPositional, int argPID)
{
    system("clear");

    if(canPrintPerProcess)
    {
        printPerProcess(root, hasPositional, argPID);
        printf("\n\n");
    }
    
    if(canPrintSystemWide)
    {
        printSystemWide(root, hasPositional, argPID);
        printf("\n\n");
    }

    if(canPrintVNodes)
    {
        printVNodes(root, hasPositional, argPID);
        printf("\n\n");
    }

    if(canPrintComposite)
    {
        printComposite(root, hasPositional, argPID);
        printf("\n\n");
    }

    if(hasThreshold)
    {
        printf("\n ### Offending Processes ###\n");
        printThresholdList(root, thresholdNum);
    }

}

// Processes flags
int main(int argc, char **argv)
{
    // Flags and errors
    int canPrintPerProcess = 0;
    int canPrintSystemWide = 0;
    int canPrintVNodes = 0;
    int canPrintComposite = 1;

    int hasThreshold = 0;
    int thresholdNum = 0;

    int hasPositional = 0;
    int argPID = -1;

    int numOfNumberArgs = 0;
    int produceError = 0;

    char errMsg[100] = "";
    char strNum[10] = "";

    // Getting all information
    processNode *root = getPID();
    processNode *traverser = root;
    populateFD(root);
    
    if(argc == 1)
    {
        // By default, prints composite table
        printToScreen(root, 0, 0, 0, 1, 0, -1, 0, -1);
    }
    else
    {
        for(int i = 1; i < argc; i++)
        {
            // Checking if its the number
            if(isNumber(argv[i]) == 0)
            {
                canPrintComposite = 0;
                if(strcmp(argv[i], "--per-process") == 0)
                {
                    canPrintPerProcess = 1;
                }
                else if(strcmp(argv[i], "--systemWide") == 0)
                {
                    canPrintSystemWide = 1;
                }
                else if(strcmp(argv[i], "--VNodes") == 0)
                {
                    canPrintVNodes = 1;
                }
                else if(strcmp(argv[i], "--composite") == 0)
                {
                    canPrintComposite = 1;
                }
                else if(strncmp(argv[i], "--threshold=", 12) == 0)
                {
                    // Copies the string after --samples=
                    // Assumes it is 10 digits or less
                    strncpy(strNum, argv[i]+12, 10);

                    // If the string after --samples= is a number, then set samples to it
                    if(isNumber(strNum) == 1)
                    {
                        thresholdNum = atoi(strNum);
                        hasThreshold = 1;
                    }
                    else
                    {
                        produceError = 1;
                        strcpy(errMsg, " : A number that is 10 digits or less must follow --threshold=");
                        break;
                    }
                }
                // Checking if valid flag
                else
                {
                    produceError = 1;
                    break;
                }
            }
            else
            {
               numOfNumberArgs++;

               if(numOfNumberArgs > 1)
               {
                    strcpy(errMsg, " : Can only have one particular PID as a positional argument");
                    produceError = 1;
                    break;
               }

               argPID = atoi(argv[i]);
               hasPositional = 1;
            }
        }

        if(produceError == 0)
        {
            printToScreen(root, canPrintPerProcess, canPrintSystemWide, canPrintVNodes, canPrintComposite, hasThreshold, thresholdNum, hasPositional, argPID);
        }
        else
        {
            printf("Invalid flag%s\n", errMsg);
        }
    }

    deleteProcessList(root);

    return 0;
}
