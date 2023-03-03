# Assignment 2 : Recreating System-Wide FD Tables

## Table of Contents

	1. Installation

	2. Configuration
		2.1 Flags
		2.2 Positional Arguments

	3. Design Choices and Explanation of Solutions
		3.1 Structs
		3.2 Helper Functions
		3.3 List Functions
		3.4 Getter Functions
		3.5 Print Functions
		3.6 main() Function

	4. Contributors

## 1. Installation

	Upon downloading this folder, compile the program within your current directory. Type the following command into the shell:

		$ make 

	This will create an executable file in the current directory called "A2Tables.out"
	
## 2. Configuration

	To run the program with default settings, type the following command into the shell:

		$ ./A2Tables

### 2.1 Flags

	To run the program with modified parameters, type the above command into the shell, as well as any of the following flags in any order seperated by spaces:

		--per-process

			Prints table that displays PID's and FD's of all processes executed and owned by the user

						
		--systemWide

			Prints table that displays PID's, FD's, and file descriptor names of all processes executed and owned by the user

		--VNodes

			Prints table that displays FD's and Inode number of all processes executed and owned by the user

		--composite

			Prints table that displays PID's, FD's, file descriptor names, and Inode number of all processes executed and owned by the user

		NOTE: If the above flags are not inputted by the user, then --composite is run by default

		--threshold=X

			Sets the threshold number to X
			At the end of printing tables, it will print PID's and FD's of all the processes owned by the user whose FD number surpasses the threshold number

		--output_TXT

			Produces a text file called "compositeTable.txt" that stores the composite table in it in plaintext

		--output_binary

			Produces a binary file called "compositeTable.bin" that stores the composite table in it in binary text

	NOTE: Each respective table can be outputted in succession by inputting multiple flags

	Valid Commands Example
		$./A2Tables.out --VNodes --composite
		$./A2Tables.out --threshold=20

### 2.2 Positional Arguments

	To display only a certain PID in tables, type exactly one valid PID in any order

	If the PID is not a process that the user owns, or is not a process that exists at all, then the table will appear empty

	Valid Command Example
		$./A2Tables.out --per-process 2439140 --VNodes

## 3. Design Choices and Explanation of Solutions

### 3.1 Structs

	struct FDNode
	struct ProcessNode
	
		These structs were created for several reasons:

			1. Due to the uncertainty in list size, I decided that a linked list is the best way to handle the data

			2. After understanding the heirarchy of the file system, I realized that every PID holds a folder of FD's, hence
			   why I chose to store a linked list of FDNode's within each ProcessNode

			3. Each node stores data needed in one place. This makes retrieving and printing data easier
			   In ProcessNodes, I stored the PID. In FDNodes, I stored the file name, file descriptor number, and inode number

### 3.2 Helper Functions

	int isNumber(char *numStr)

		This helper function is used to check if a string is a number (ie, consists of only digits)
		It was created because checking if a string is a number is repeatedly done when reading flags or files

### 3.3 List Functions

	processNode *createProcessNode(int PID)
	FDNode *createFDNode(int FD, char filename[STR_LEN], long inode)

		Explanation of Solution:
			These list functions are standard node creation functions
			They dynamically allocate memory for their respective nodes, and initialize all fields of the node using data passed into the parameters
		
			For processNodes, their linked list of FDNodes is initialized as NULL.
			This is because it will be later populated using the populateFD() function discussed in section 3.4

	processNode *insertProcessList(processNode *root, int PID)
	FDNode *insertFDList(FDNode *root, int FD, char filename[STR_LEN], long inode)

		Explanation of Solution:
			These list functions are standard node insertion functions
			They create a new node using the data passed into the parameters, and insert it at the end of the exist linked list beginning at root

			They insert at the end to ensure that PID and FD numbers are in increasing order

	FDNode *deleteFDList(FDNode *root)
	processNode *deleteProcessList(processNode *root)

		Explanation of Solution:
			These list functions are standard node deletion functions
			They delete the whole linked list

			Since each processNode has a linked list of FDNodes, deleteFDList() is called within deleteProcessList() for each processNode
			This is to ensure that the linked list it stores is freed before it itself is freed

### 3.4 Getter Functions

	FDNode *getFD(int PID)
	
		This function takes in a PID

		Explanation of Solution:	
			Using the PID passed into the parameter, this function traverses through the /proc/[PID]/fd/ directory to generate a linked list of FDNodes
		
			As it traverses through the folder, it:

				1. Retrieves the File Descriptor number from the file name

				2. Retrieves the File Descriptor name using readlink() from the unistd.h library

				3. Retrieves the inode number using stat() from the sys/stat.h library

			Using the information retrieved, it calls insertFDList() to create/append to the linked list of FDNodes

			This linked list of FDNodes is returned

	processNode *getPID()

		This function takes nothing

		Explanation of Solution:
			This function traverses through the /proc/ directory to generate a linked list of processNodes
		
			As it traverses through the folder, it:

				1. Checks if the process is owned by the user by comparing user ID's

				2. Retrieves the PID number from the file name

			Using the information retrieved, it calls insertProcessList() to create/append to the linked list of processNodes

			This linked list of processNodes is returned

	void populateFD(processNode *root)
		
		This function takes in a linked list of processNodes		

		Explanation of Solution:
			This function traverses through the linked list of processNodes, and uses getFD() to create a linked list
			of FDNodes corresponding to that PID

			The original linked list of processNodes is returned, with their linked list of FDNodes now populated

### 3.5 Print Functions

	void printPerProcess(processNode *root, int hasPositional, int argPID)

		This function takes in a linked list of processNodes, hasPositional, and argPID

		Explanation of Solution:
			This function prints out the PID and File Descriptor number in a table by traversing the linked list of processNodes
			This is because the linked list of processNodes has all the information we need by how I designed the CDT

	void printSystemWide(processNode *root, int hasPositional, int argPID)
	
		This function takes in a linked list of processNodes, hasPositional, and argPID

		Explanation of Solution:
			This function prints out the PID, File Descriptor number, and the File Descriptor name in a table by traversing the linked list of processNodes
			This is because the linked list of processNodes has all the information we need by how I designed the CDT

	void printVNodes(processNode *root, int hasPositional, int argPID)

		This function takes in a linked list of processNodes, hasPositional, and argPID

		Explanation of Solution:
			This function prints out the File Descriptor number and inode number in a table by traversing the linked list of processNodes
			This is because the linked list of processNodes has all the information we need by how I designed the CDT
	
	void printComposite(processNode *root, int hasPositional, int argPID)

		This function takes in a linked list of processNodes, hasPositional, and argPID

		Explanation of Solution:
			This function prints out the PID, File Descriptor number, File Descriptor name, and inode number in a table by traversing the linked list of processNodes
			This is because the linked list of processNodes has all the information we need by how I designed the CDT

			If the user uses --output_TXT or --output_BIN, then this function is also responsible for creating and writing the contents of
			the composite table onto their respective files
	
	void printThresholdList(processNode *root, int thresholdNum, int hasPositional, int argPID)

		This function takes in a linked list of processNodes, thresholdNum, hasPositional, and argPID
		
		Explanation of Solution:
			This function is only called if a threshold flag was given, and only after every table has been already printed

			It seperates itself from tables by first displaying:
		
				### Offending Processes ###
			
			It traverses the linked list of processNodes and the linked list of FDNodes within them, and will print
			the PID and File Descriptor number of processes whose file descriptor numbers are greater than thresholdNum
			
	NOTE: hasPositional and argPID is passed in due to the positional argument requirement
	      If a positional argument is entered, then this function will check whether or not the PID of the processNodes match the argPID before printing to the table

	void printToScreen(processNode *root, int canPrintPerProcess, int canPrintSystemWide, int canPrintVNodes, int canPrintComposite, int hasThreshold, int thresholdNum, int hasPositional, int argPID, int hasOutputTXT, int hasOutputBIN)

		This function takes in many variables, one of which stores the linked list of processNodes, as well as variables representing the flags passed in as command-line arguments

		Explanation of Solution:
			This function controls which tables and information are printed to the screen (and text/binary files) using the variables passed in
			The value of these variables depend on whether or not its respective flag was encountered in argv in main()

### 3.5 main() Function

	int main(int argc, char **argv)

		This function takes in argc (number of command-line arguments) and argv (an array of command-line arguments)

		Explanation of Solution:
			This function is responsible for creating a linked list of processNodes, then populating the FDNodes within each processNode

			This function is also responsible for iterating through all command-line arguments

			This function is also responsible for checking if flags are in the right spelling and format
			If any flag is invalid, an error message will be printed on screen and the program will not be run

			If there are no flags, or only a positional argument or threshold flag are present, then 
			printToScreen() will be called to display the composite table (which is displayed by default)

			If there are other flags and all flags are valid, then printToScreen() is called using modified values based on the flags

			After printToScreen() has completed, then all linked lists are freed to avoid memory leaks

## 4. Contributors

Ashtian Dela Cruz
ashtian.delacruz@mail.utoronto.ca
1008154710 

