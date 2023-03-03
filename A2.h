
#define STR_LEN 1024

/*
    CDT's
*/
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
    LIST FUNCTIONS
*/

/*
    These list functions are standard node creation functions
	They dynamically allocate memory for their respective nodes, and initialize all fields of the node using data passed into the parameters
*/
processNode *createProcessNode(int PID);
FDNode *createFDNode(int FD, char filename[STR_LEN], long inode);

/*
    These list functions are standard node insertion functions
	They create a new node using the data passed into the parameters, and insert it at the end of the exist linked list beginning at root
*/
processNode *insertProcessList(processNode *root, int PID);
FDNode *insertFDList(FDNode *root, int FD, char filename[STR_LEN], long inode);

/*
    These list functions are standard node deletion functions
	They delete the whole linked list
*/
FDNode *deleteFDList(FDNode *root);
processNode *deleteProcessList(processNode *root);

/*
    MAIN FUNCTIONS
*/

/*
    Using the PID passed into the parameter, this function traverses 
    through the /proc/[PID]/fd/ directory to generate a linked list of FDNodes
*/
FDNode *getFD(int PID);

/*
    This function traverses through the /proc/ directory to generate a linked list of processNodes
*/
processNode *getPID();

/*
    This function traverses through the linked list of processNodes, and uses getFD() to create a linked list
	of FDNodes corresponding to that PID
*/
void populateFD(processNode *root);

/*
    This function prints out the PID and File Descriptor number in a table
*/
void printPerProcess(processNode *root, int hasPositional, int argPID);

/*
    This function prints out the PID, File Descriptor number, and the File Descriptor name in a table
*/
void printSystemWide(processNode *root, int hasPositional, int argPID);

/*
    This function prints out the File Descriptor number and inode number in a table
*/
void printVNodes(processNode *root, int hasPositional, int argPID);

/*
    This function prints out the PID, File Descriptor number, File Descriptor name, and inode number in a table
*/
void printComposite(processNode *root, int hasPositional, int argPID, int hasOutputTXT, int hasOutputBIN);

/*
    It traverses the linked list of processNodes and the linked list of FDNodes within them, and will print
	the PID and File Descriptor number of processes whose file descriptor numbers are greater than thresholdNum
*/
void printThresholdList(processNode *root, int thresholdNum, int hasPositional, int argPID);

/*
    This function controls which tables and information are printed to the screen (and text/binary files) using the variables passed in
*/
void printToScreen(processNode *root, int canPrintPerProcess, int canPrintSystemWide, int canPrintVNodes, int canPrintComposite, int hasThreshold, int thresholdNum, int hasPositional, int argPID, int hasOutputTXT, int hasOutputBIN);