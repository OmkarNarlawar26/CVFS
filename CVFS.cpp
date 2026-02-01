//////////////////////////////////////////////////////////////////////////////////
//
//  Header File Inclusion
//
//////////////////////////////////////////////////////////////////////////////////

#include<stdio.h>    // For printf, fgets, etc.
#include<stdlib.h>   // For malloc, free, atoi
#include<unistd.h>   // For system level functions (future use)
#include<stdbool.h>  // For bool, true, false
#include<string.h>   // For strcpy, strcmp, strncpy, memset

//////////////////////////////////////////////////////////////////////////////////
//
//  User Defined Macros
//
//////////////////////////////////////////////////////////////////////////////////

//  Maximum file size that we allow in the project
#define MAXFILESIZE 50     // Maximum bytes allowed in one file
#define MAXOPENFILES 20    // Maximum files that can be opened at a time
#define MAXINODE 5         // Maximum number of files (inodes)

#define READ 1             // Permission bit for read
#define WRITE 2            // Permission bit for write
#define EXECUTE 4          // Permission bit for execute (not used yet)

#define START 0            // For lseek like operations (future use)
#define CURRENT 1
#define END 2

#define EXECUTE_SUCCESS 0  // Generic success return value

#define REGULARFILE 1      // File is valid and created
#define SPECIALFILE 2      // Reserved for future use

//////////////////////////////////////////////////////////////////////////////////
//
//  User Defined Macros For Error Handling
//
//////////////////////////////////////////////////////////////////////////////////

#define ERR_INVALID_PARAMETER -1

#define ERR_NO_INODES -2

#define ERR_FILE_ALREADY_EXIST -3
#define ERR_FILE_NOT_EXIST -4

#define ERR_PERMISSION_DENIED -5

#define ERR_INSUFFICIENT_SPACE -6
#define ERR_INSUFFICIENT_DATA -7

#define ERR_MAX_FILES_OPEN -8

//////////////////////////////////////////////////////////////////////////////////
//
//  User Defined Structures
//
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :            BootBlock
//  Description  :              Holds the information to boot the OS
//
//////////////////////////////////////////////////////////////////////////////////

struct BootBlock
{
    char Information[100];   // Stores boot message of CVFS
};

//////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :            SuperBlock
//  Description  :              Holds the information about the File System
//
//////////////////////////////////////////////////////////////////////////////////

struct SuperBlock
{
    int TotalInodes;  // Total files possible
    int FreeInodes;   // How many are still unused
};

//////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :            Inode
//  Description  :              Holds the information about File
//
//////////////////////////////////////////////////////////////////////////////////

#pragma pack(1)
struct Inode
{
    char FileName[20];     // Name of file
    int InodeNumber;       // Unique id
    int FileSize;          // Max allowed size (MAXFILESIZE)
    int ActualFileSize;    // Current used size
    int FileType;          // 0 = free, 1 = regular file
    int ReferenceCount;    // How many times file is opened
    int Permission;        // READ / WRITE / READ+WRITE
    char *Buffer;          // Actual data of file (Actual File allocation Address)
    struct Inode *next;    // Next inode in linked list
};

typedef struct Inode INODE;
typedef struct Inode* PINODE;
typedef struct Inode** PPINODE;

//////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :            FileTable
//  Description  :              Holds the information about Opened File
//
//////////////////////////////////////////////////////////////////////////////////

struct FileTable
{
    int ReadOffset;     // Where next read will start
    int WriteOffset;    // Where next write will start
    int Mode;           // Open mode
    PINODE ptrinode;    // Pointer to its inode (Pointer created for Inode)
};

typedef FileTable FILETABLE;
typedef FileTable * PFILETABLE;

//////////////////////////////////////////////////////////////////////////////////
//
//  Structure Name :            UAREA
//  Description  :              Holds the information about Process
//
//////////////////////////////////////////////////////////////////////////////////

struct UAREA
{
    char ProcessName[20];               // Name of running process
    PFILETABLE UFDT[MAXOPENFILES];      // FileTable *  UFDT is Array and MAXOPENFILES is size(for Ex : 30)
                                        // Array of open files
};

//////////////////////////////////////////////////////////////////////////////////
//
//  Global variables or objects used in the Project
//
//////////////////////////////////////////////////////////////////////////////////

BootBlock bootobj;
SuperBlock superobj;
UAREA uareaobj;

PINODE head = NULL;

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :     InitialiseUAREA
//  Description :       This function prepares the UAREA (User Area) structure for use.
//                      It sets the default process name and marks all entries in the
//                      User File Descriptor Table (UFDT) as NULL, which means no files
//                      are opened at the start of the system.
//                      It is called once during system boot.
//  Effect :            - Sets process name to "Myexe"
//                      - Makes all UFDT entries empty (no open files)
//  Author :            Omkar Sachin Narlawar
//  Date :              13/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

void InitialiseUAREA()
{
    strcpy(uareaobj.ProcessName,"Myexe");

    int i = 0;

    for(i = 0; i < MAXOPENFILES; i++)
    {
        uareaobj.UFDT[i] = NULL;        // No file opened initially
    }

    printf("Omkar's CVFS : UAREA gets initialised successfully\n");
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         InitialiseSuperBlock
//  Description :           This function initializes the SuperBlock which keeps
//                          track of total and free inodes (files) in the file
//                          system.
//                          At the beginning all inodes are free.
//  Author :                Omkar Sachin Naralwar
//  Date :                  13/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

void InitialiseSuperBlock()
{
    superobj.TotalInodes = MAXINODE;
    superobj.FreeInodes = MAXINODE;

    printf("Omkar's CVFS : Super block gets initialised successfully\n");
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         CreateDILB
//  Description :           This function creates the Disk Inode List Block (DILB)
//                          as a linked list of inodes. Each inode represents a
//                          possible file. Initially all inodes are marked as free
//                          and have no data buffer.
//  Effect :                - Allocates MAXINODE inode nodes
//                          - Links them into a singly linked list
//                          - Marks each inode as unused (FileType = 0)
//  Author :                Omkar Sachin Naralwar
//  Date :                  13/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    for(i = 1; i <= MAXINODE; i++)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        strcpy(newn->FileName,"\0");
        newn->InodeNumber = i;
        newn->FileSize = 0;
        newn->ActualFileSize = 0;
        newn->FileType = 0;
        newn->ReferenceCount = 0;
        newn->Permission = 0;
        newn->Buffer = NULL;
        newn->next = NULL;

        if(temp == NULL)            // LL is Empty
        {
            head = newn;
            temp = head;
        }
        else                        // LL contains atleast one node
        {
            temp->next = newn;
            temp = temp->next;
        }
    }

    printf("Omkar's CVFS : DILB created successfully\n");
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         StartAuxillaryDataInitialisation
//  Description :           It is used to call all such functions which are 
//                          used to initialise auxillary data.
//                          This is the master initialization routine of CVFS.
//                          It sets the boot message and calls all other
//                          initialization functions required before the file
//                          system starts.
//  Calls internally :      - InitialiseSuperBlock()
//                          - CreateDILB()
//                          - InitialiseUAREA()
//  Effect :                The virtual file system becomes ready to accept commands.                        
//  Author :                Omkar Sachin Naralwar
//  Date :                  13/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

void StartAuxillaryDataInitialisation()
{
    strcpy(bootobj.Information,"Booting process of Omkar's CVFS is done");

    printf("%s\n",bootobj.Information);

    InitialiseSuperBlock();

    CreateDILB();

    InitialiseUAREA();

    printf("Omkar's CVFS : Auxillary data initialise successfully\n");
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         DisplayHelp
//  Description :           Displays a help menu showing all supported shell
//                          commands and their basic purpose so that the user
//                          can understand how to use the CVFS shell.
//  Output :                List of commands like ls, creat, read, write, unlink,
//                          exit, etc.
//  Author :                Omkar Sachin Naralwar
//  Date :                  14/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

void DisplayHelp()
{
    printf("--------------------------------------------------------------------\n");
    printf("---------------------Omkar's CVFS Help Page----------------------\n");
    printf("--------------------------------------------------------------------\n");
    printf("\n");
    
    printf("man     : It is used to display manual page\n");
    printf("ls      : List all files with details\n");
    printf("clear   : It is used to clear the terminal\n");
    printf("creat   : It is used to create new file\n");
    printf("write   : It is used to write the data into file\n");
    printf("read    : It is used to read the data from the file\n");
    printf("stat    : It is used to display statistical information\n");
    printf("unlink  : It is used to delete the file\n");
    printf("exit    : It is use to terminate Omkar's CVFS\n");
    
    printf("\n");
    printf("--------------------------------------------------------------------\n");
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         ManPageDisplay
//  Description :           This function displays detailed manual
//                          information of a specific command similar to
//                          the Linux 'man' command.
//  Input :                 Name of the command whose manual is required.
//  Author :                Omkar Sachin Naralwar
//  Date :                  14/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

void ManPageDisplay(char Name[])
{
    if(strcmp("ls",Name) == 0)
    {
        printf("About        : It is used to list the names of all files\n");
        printf("Usage        : ls\n");
    }
    else if(strcmp("man",Name) == 0)
    {
        printf("About        : It is used to Display manual page\n");
        printf("Usage        : man command_name\n");
        printf("command_name : It is the name of command\n");
    }
    else if(strcmp("exit",Name) == 0)
    {
        printf("About        : It is used to terminate the shell\n");
        printf("Usage        : exit\n");
    }
    else if(strcmp("clear",Name) == 0)
    {
        printf("About        : It is used to clear the shell\n");
        printf("Usage        : clear\n");
    }
    else if(strcmp("creat",Name) == 0)
    {
        printf("About        : It is used to create the new file\n");
        printf("Usage        : creat\n");
    }
    else if(strcmp("unlink",Name) == 0)
    {
        printf("About        : It is used to delete the file\n");
        printf("Usage        : unlink\n");
    }
    else
    {
        printf("No manual entry for %s\n",Name);
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         IsFileExist
//  Description :           It is used to check whether file is already exist or not
//  Input :                 It accepts file name,
//                          File name to be searched.
//  Output :                true  -> file is present
//                          false -> file is not present
//  Author :                Omkar Sachin Naralwar
//  Date :                  16/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

bool IsFileExist(
                    char *name            //  File name
                )
{
    PINODE temp = head;
    bool bFlag = false;

    while(temp != NULL)
    {
        if((strcmp(name,temp->FileName) == 0) && (temp->FileType == REGULARFILE))
        {
            bFlag = true;
            break;
        }
        temp = temp->next;
    }

    return bFlag;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         CreateFile
//  Description :           This function creates a new regular file
//  Working :               - Validates parameters
//                          - Checks free inode availability
//                          - Checks duplicate file name
//                          - Allocates inode and file table
//                          - Allocates memory buffer for file data
//                          - Updates super block information
//  Input :                 It accepts -
//                                   name        -> Name of file
//                                   permission  -> 1(Read),2(Write),3(Read+Write)
//  Output :                It returns File descriptor on success
//                          Error code on failure
//  Author :                Omkar Sachin Naralwar
//  Date :                  16/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

int CreateFile(
                    char *name,             // Name of new file
                    int permission          // Permission for that file
                )
{
    PINODE temp = head;
    int i = 0;

    printf("Total number of Inodes remaining : %d\n",superobj.FreeInodes);

    //  If name is missing
    if(name == NULL)
    {
        return ERR_INVALID_PARAMETER;
    }

    //  If the permission value is wrong
    //  permission -> 1 -> READ
    //  permission -> 2 -> WRITE
    //  permission -> 3 -> READ + WRITE
    if(permission < 1 || permission > 3)
    {
        return ERR_INVALID_PARAMETER;
    }

    //  If the inodes are full
    if(superobj.FreeInodes == 0)
    {
        return ERR_NO_INODES;
    }

    //  If file is already present
    if(IsFileExist(name) == true)
    {
        return ERR_FILE_ALREADY_EXIST;
    }

    //  Search empty Inode
    while(temp != NULL)
    {
        if(temp->FileType == 0)
        {
            break;
        }
        temp = temp->next;
    }

    if(temp == NULL)            //  If temp reached to last node
    {
        printf("There is no Inode");
        return ERR_NO_INODES;
    }

    //  Search for empty UDFT entry
    // Note : 0 1 2 are reserved
    for(i = 3; i < MAXOPENFILES ; i++)
    {
        if(uareaobj.UFDT[i] == NULL)
        {
            break;
        }
    }

    //  UFDT is full
    if(i == MAXOPENFILES)
    {
        return ERR_MAX_FILES_OPEN;
    }

    //  Allocate memory for file table
    uareaobj.UFDT[i] = (PFILETABLE)malloc(sizeof(FILETABLE));

    //  Initialise File Table
    uareaobj.UFDT[i]->ReadOffset = 0;
    uareaobj.UFDT[i]->WriteOffset = 0;
    uareaobj.UFDT[i]->Mode = permission;

    //  Connect File Table with Inode
    uareaobj.UFDT[i]->ptrinode = temp;

    //  Initialise elements of Inode
    strcpy(uareaobj.UFDT[i]->ptrinode->FileName,name);
    uareaobj.UFDT[i]->ptrinode->FileSize = MAXFILESIZE;
    uareaobj.UFDT[i]->ptrinode->ActualFileSize = 0;
    uareaobj.UFDT[i]->ptrinode->FileType = REGULARFILE;
    uareaobj.UFDT[i]->ptrinode->ReferenceCount = 1;
    uareaobj.UFDT[i]->ptrinode->Permission = permission;

    //  Allocate memory for files data
    uareaobj.UFDT[i]->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

    superobj.FreeInodes--;

    return i;           // File descriptor
    
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         LsFile
//  Description :           This function traverses the inode list and
//                          displays information of all existing files.
//  Displayed Details :     Inode number, File name and Actual file size.
//  Purpose :               To provide file listing similar to 'ls -l'.
//  Author :                Omkar Sachin Naralwar
//  Date :                  16/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

//  ls -l
void LsFile()
{
    PINODE temp = head;

    printf("--------------------------------------------------------------------\n");
    printf("-----------------Omkar's CVFS Files Information------------------\n");
    
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%d\t%s\t%d\n",temp->InodeNumber,temp->FileName,temp->ActualFileSize);
        }
        
        temp = temp->next;
    }

    printf("--------------------------------------------------------------------\n");
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         UnlinkFile()
//  Description :           This function deletes an existing file from
//                          the virtual file system.
//  Working :               - Finds matching file in UFDT
//                          - Frees file data buffer
//                          - Resets inode metadata
//                          - Frees file table entry
//                          - Increments free inode count
//  Input :                 Name of file to be deleted.
//  Return :                EXECUTE_SUCCESS on success
//                          Error code on failure
//  Author :                Omkar Sachin Naralwar
//  Date :                  22/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

int UnlinkFile(
                    char *name
                )
{
    int i = 0;

    if(name == NULL)
    {
        return ERR_INVALID_PARAMETER;
    }

    if(IsFileExist(name) == false)
    {
        return ERR_FILE_NOT_EXIST;
    }

    //  Travel the UDFT
    for(i = 0; i < MAXOPENFILES; i++)
    {
        if(uareaobj.UFDT[i] != NULL)
        {
            if(strcmp(uareaobj.UFDT[i]->ptrinode->FileName, name) == 0)
            {
                //  Deallocate memory of Buffer
                free(uareaobj.UFDT[i]->ptrinode->Buffer);
                uareaobj.UFDT[i]->ptrinode->Buffer = NULL;

                // Reset all values of inode
                // Dont deallocate memory of inode
                uareaobj.UFDT[i]->ptrinode->FileSize = 0;
                uareaobj.UFDT[i]->ptrinode->ActualFileSize = 0;
                uareaobj.UFDT[i]->ptrinode->FileType = 0;
                uareaobj.UFDT[i]->ptrinode->ReferenceCount = 0;
                uareaobj.UFDT[i]->ptrinode->Permission = 0;

                memset(uareaobj.UFDT[i]->ptrinode->FileName, '\0', sizeof(uareaobj.UFDT[i]->ptrinode->FileName));

                //  Deallocate memory of file table
                free(uareaobj.UFDT[i]);

                //  Set NULL to UFDT
                uareaobj.UFDT[i] = NULL;

                //  Increment free inodes count
                superobj.FreeInodes++;

                break;      // break is of for loop (IMP)

            }   //  End of if
        }       //  End of if
    }           //  End of for

    return EXECUTE_SUCCESS;
}               //  End of Function

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         WriteFile()
//  Description :           This function writes given data into the file
//                          associated with the provided file descriptor.
//  Checks :                Valid FD, write permission and available space.
//  Effect :                Copies data into file buffer and updates
//                          write offset as well as actual file size.
//  Input :                 fd   -> File descriptor
//                          data -> Source buffer(Address of buffer which contains data)
//                          size -> Size of data that we want to write
//  Output :                Number of bytes successfully written or error code.
//  Author :                Omkar Sachin Naralwar
//  Date :                  22/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

int WriteFile(
                    int fd,
                    char *data,
                    int size
            )
{
    printf("File Descriptor : %d\n",fd);
    printf("Data that we want to write : %s\n",data);
    printf("Number of bytes that we want to write : %d\n",size);

    //  Invalid FD
    if(fd < 0 || fd > MAXOPENFILES)
    {
        return ERR_INVALID_PARAMETER;
    }

    //  FD points to NULL
    if(uareaobj.UFDT[fd] == NULL)
    {
        return ERR_FILE_NOT_EXIST;
    }

    //  There is no permission to write
    if(uareaobj.UFDT[fd]->ptrinode->Permission < WRITE)
    {
        return ERR_PERMISSION_DENIED;
    }

    //  Insufficient space
    if((MAXFILESIZE - uareaobj.UFDT[fd]->WriteOffset) < size)
    {
        return ERR_INSUFFICIENT_SPACE;
    }

    //  Write the data into the file
    strncpy(uareaobj.UFDT[fd]->ptrinode->Buffer + uareaobj.UFDT[fd]->WriteOffset, data, size);

    //  Update the write offset
    uareaobj.UFDT[fd]->WriteOffset = uareaobj.UFDT[fd]->WriteOffset + size;

    // Update the actual file size
    uareaobj.UFDT[fd]->ptrinode->ActualFileSize = uareaobj.UFDT[fd]->ptrinode->ActualFileSize + size;

    return size;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         ReadFile()
//  Description :           This function reads data from the file associated
//                          with the given file descriptor into user buffer.
//  Checks :                Valid FD, read permission and available data.
//  Effect :                Copies data from file buffer to user buffer and
//                          updates read offset.
//  Input :                 fd   -> File descriptor
//                          data -> Destination buffer (Address of empty Buffer)
//                          size -> Number of bytes to read
//  Output :                Number of bytes successfully read or error code
//  Author :                Omkar Sachin Naralwar
//  Date :                  22/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

int ReadFile(
                int fd,
                char *data,
                int size
            )
{
    //  Invalid FD
    if(fd < 0 || fd >MAXOPENFILES)
    {
        return ERR_INVALID_PARAMETER;
    }
      
    if(data == NULL)
    {
        return ERR_INVALID_PARAMETER;
    }

    if(size <= 0)
    {
        return ERR_INVALID_PARAMETER;
    }

    //  File not found
    if(uareaobj.UFDT[fd] == NULL)
    {
        return ERR_FILE_NOT_EXIST;
    }

    //  Filter for permission
    if(uareaobj.UFDT[fd]->ptrinode->Permission < READ)
    {
        return ERR_PERMISSION_DENIED;
    }

    //  Insufficient data
    if((MAXFILESIZE - uareaobj.UFDT[fd]->ReadOffset) < size)
    {
        return ERR_INSUFFICIENT_DATA;
    }

    //  Read the data
    strncpy(data, uareaobj.UFDT[fd]->ptrinode->Buffer + uareaobj.UFDT[fd]->ReadOffset, size);

    //  Update the read offset
    uareaobj.UFDT[fd]->ReadOffset = uareaobj.UFDT[fd]->ReadOffset + size;

    return size;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  Entry Point Function of the Project
//
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//
//  Function Name :         main
//  Description  :          Entry point of Omkar's CVFS shell.
//                          It initialises the system and continuously
//                          accepts user commands to perform file
//                          operations like create, read, write, delete,
//                          list files etc.
//  Working :               - Initialises auxiliary data
//                          - Displays startup banner
//                          - Runs infinite command processing loop
//                          - Parses and executes user commands
//                          - Terminates only when user enters 'exit'
//  Author :                Omkar Sachin Naralwar
//  Date :                  22/01/2026
//
//////////////////////////////////////////////////////////////////////////////////

int main()
{
    char str[80] = {'\0'};                 // Stores complete command entered by user
    char Command[5][20] = {{'\0'}};        // Stores separated words of command
    char InputBuffer[MAXFILESIZE] = {'\0'};// Buffer used for write operation

    char *EmptyBuffer = NULL;              // Dynamic buffer for read operation

    int iCount = 0;                        // Number of words entered in command
    int iRet = 0;                          // Stores return value of functions

    //  Initialise all system data structures
    StartAuxillaryDataInitialisation();

    printf("\n");
    printf("--------------------------------------------------------------------\n");
    printf("----------------Omkar's CVFS started Successfully----------------\n");
    printf("--------------------------------------------------------------------\n");

    // Infinite listening Shell
    while(1)
    {
        fflush(stdin);                  // Clear input buffer

        strcpy(str,"");                 // Reset command string

        // Display shell prompt
        printf("\nOmkar's CVFS : > ");

        // Accept full command line from user
        fgets(str,sizeof(str),stdin);

        // Split command into maximum 5 words
        iCount = sscanf(str,"%s %s %s %s %s",Command[0],Command[1],Command[2],Command[3],Command[4]);

        fflush(stdin);

        ////////////////////////////////////////////////////////////////////////
        // Commands with only 1 word
        ////////////////////////////////////////////////////////////////////////
        if(iCount == 1)
        {
            //  exit command : terminates CVFS
            //  Omkar's CVFS : > exit
            if(strcmp("exit",Command[0]) == 0)
            {
                printf("Thank you for using Omkar's CVFS\n");
                printf("Deallocating all the allocated resources\n");

                break;
            }

            //  ls command : list all files
            //  Omkar's CVFS : > ls
            else if(strcmp("ls",Command[0]) == 0)
            {
                LsFile();
            }

            //  help command : display help page
            //  Omkar's CVFS : > help
            else if(strcmp("help",Command[0]) == 0)
            {
                DisplayHelp();
            }

            //  clear command : clear terminal screen
            //  Omkar's CVFS : > clear
            else if(strcmp("clear",Command[0]) == 0)
            {
                #ifdef _WIN32
                    system("cls");
                #else
                    system("clear");
                #endif
            }
            else
            {
                printf("Command not Found\n");
                printf("Please refer help option to get more information\n");
            }
        }   // End of else if 1

        ////////////////////////////////////////////////////////////////////////
        // Commands with 2 words
        ////////////////////////////////////////////////////////////////////////
        else if(iCount == 2)
        {
            //  man command : show manual page of given command
            //  Omkar's CVFS : > man ls
            if(strcmp("man",Command[0]) == 0)
            {
                ManPageDisplay(Command[1]);
            }

            //  unlink command : delete given file
            //  Omkar's CVFS : > unlink Demo.txt
            else if(strcmp("unlink",Command[0]) == 0)
            {
                iRet = UnlinkFile(Command[1]);

                if(iRet == ERR_INVALID_PARAMETER)
                {
                    printf("Error : Invalid parameter");
                }

                if(iRet == ERR_FILE_NOT_EXIST)
                {
                    printf("Error :  Unable to delete as there is no such file");
                }

                if(iRet == EXECUTE_SUCCESS)
                {
                    printf("File gets successfully deleted\n");
                }
            }

            //  write command : write data into file using FD
            //  Omkar's CVFD : > write 2   (here 2 is considered as fd)
            else if(strcmp("write",Command[0]) == 0)
            {
                printf("Enter the data that you want to write : \n");

                // Accept data from user
                fgets(InputBuffer,MAXFILESIZE,stdin);

                // Perform write operation
                iRet = WriteFile(atoi(Command[1]), InputBuffer, strlen(InputBuffer)-1);

                if(iRet == ERR_INVALID_PARAMETER)
                {
                    printf("Error : Invalid parameters\n");
                }
                else if(iRet == ERR_FILE_NOT_EXIST)
                {
                    printf("Error : There is no such file\n");
                }
                else if(iRet == ERR_PERMISSION_DENIED)
                {
                    printf("Error : Unable to write as there is no permission\n");
                }
                else if(iRet == ERR_INSUFFICIENT_DATA)
                {
                    printf("Error : Unable to write as there us no space\n");
                }
                else
                {
                    printf("%d bytes gets successfully wrriten\n",iRet);
                }
            }
            else
            {
                printf("Command not Found\n");
                printf("Please refer help option to get more information\n");
            }
        }   // End of else if 2

        ////////////////////////////////////////////////////////////////////////
        // Commands with 3 words
        ////////////////////////////////////////////////////////////////////////
        else if(iCount == 3)
        {
            //  creat command : create new file with permission
            //  Omkar's CVFS : > creat Ganesh.txt 3
            if(strcmp("creat",Command[0]) == 0)
            {
                iRet = CreateFile(Command[1],atoi(Command[2]));     // atoi is ascii to integer 

                if(iRet == ERR_INVALID_PARAMETER)
                {
                    printf("Error : Unable to create the file as parameters are invalid\n");
                    printf("Please refer man page\n");
                }

                if(iRet == ERR_NO_INODES)
                {
                    printf("Error : Unable to create file as there is no inode\n");
                }

                if(iRet == ERR_FILE_ALREADY_EXIST)
                {
                    printf("Error : Unable to create file because the file is already present\n");
                }

                if(iRet == ERR_MAX_FILES_OPEN)
                {
                    printf("Error : Unable to create file\n");
                    printf("Max opened files limit reached\n");
                }

                else if(iRet >= 0)
                {
                    printf("File gets successfully created with FD %d\n",iRet);
                }
            }
            //  read command : read given number of bytes from file
            //  Omkar's CVFS : > read 3 10
            else if(strcmp("read",Command[0]) == 0)
            {
                // Allocate memory for read buffer
                EmptyBuffer = (char*)malloc(sizeof(atoi(Command[2])));

                // Perform read operation
                iRet = ReadFile(atoi(Command[1]), EmptyBuffer, atoi(Command[2]));

                if(iRet == ERR_INVALID_PARAMETER)
                {
                    printf("Error : Invalid Parameter\n");
                }
                else if(iRet == ERR_FILE_NOT_EXIST)
                {
                    printf("Error : File not exist\n");
                }
                else if(iRet == ERR_PERMISSION_DENIED)
                {
                    printf("Error : Permission denied\n");
                }
                else if(iRet == ERR_INSUFFICIENT_DATA)
                {
                    printf("Error : Insufficient data\n");
                }
                else
                {
                    EmptyBuffer[iRet] = '\0';     // Make it proper string
                    printf("Read operation is successful\n");
                    printf("Data from file is : %s\n",EmptyBuffer);

                    free(EmptyBuffer);           // Free allocated buffer
                }
            }
            else
            {
                printf("Command not Found\n");
                printf("Please refer help option to get more information\n");
            }
        }   // End of else if 3
        else if(iCount == 4)
        {

        }   // End of else if 4
        else
        {
            printf("Command not Found\n");
            printf("Please refer help option to get more information\n");
        }   // End of else 
    }   // End of While

    return 0;
}   // End of while
