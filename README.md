

<h1 align="center">Customised Virtual File System (CVFS)</h1>

---

## Technology Used

The project is developed using the **C programming language**. C is chosen because it provides low-level memory management, pointer manipulation, and close-to-hardware control, which are essential to understand how operating systems and file systems work internally.

    # Concepts Used:

    File System Internals

    Linked List

    Structures

    Dynamic Memory Allocation

    Pointers

    Command Line Interface (Shell)

    Operating System Concepts

---

## User Interface Used

The project uses a **Command Line Interface (CLI)**, also known as a shell-based interface. Users interact with the virtual file system by typing commands such as `creat`, `read`, `write`, `ls`, `unlink`, etc. This interface closely resembles UNIX/Linux shell behavior and helps in understanding real OS command execution.

---

## Platform Required

* Operating System: **Linux / UNIX / Windows (with GCC compiler(using GCC / MinGW / WSL))**
* Compiler: **GCC (GNU Compiler Collection)**
  The project is platform-independent at the logic level, as it runs in user space and does not depend on actual disk hardware.

---

## Hardware Requirements

* Processor: Any modern processor Intel/AMD (Minimum Dual Core)
* RAM: Minimum **2 GB** (4 GB recommended)
* Storage: Very minimal, At least 50 MB free disk space (project works in memory only)
* Architecture: 32-bit or 64-bit

---

## Description of the Project

The **Customised Virtual File System (CVFS)** is a simulated file system implemented entirely in main memory (RAM). It mimics the behavior of a real operating system file system such as Linux EXT or FAT, but without interacting with the actual hard disk.

The main goal of this project is **educational**. It helps understand:

* How files are created, opened, read, written, and deleted
* How operating systems manage file metadata
* How inodes, file tables, and user file descriptor tables work internally

The project implements a **single-level file system**, meaning all files exist at the same level (no directories). Each file has a name, permissions, size, and a data buffer allocated dynamically in memory.

All operations such as file creation, deletion, reading, and writing are performed on virtual structures, not on real files of the OS.

---

## Data Structures Used in the Project

### 1) Boot Block

The Boot Block stores information related to the startup of the virtual file system. It contains a simple boot message that confirms the CVFS has started successfully.

**Purpose:**

* Represents OS boot information
* Displays initialization confirmation

---

### 2) Super Block

The Super Block holds **global information** about the file system.

**Key Responsibilities:**

* Stores the total number of inodes
* Keeps track of free (unused) inodes

This structure helps the system decide whether a new file can be created or not.

---

### 3) Inode (Index Node)

An inode represents **one file** in the system. Every file has exactly one inode.

**Information stored in inode:**

* File name
* Inode number (unique ID)
* Maximum file size
* Actual file size
* File type (free or regular)
* Reference count (how many times the file is opened)
* Permissions (read/write)
* Pointer to actual data buffer
* Pointer to next inode (linked list)

All inodes together form a **linked list**, called the Disk Inode List Block (DILB).

---

### 4) File Table

The File Table stores information about **opened files**.

**Responsibilities:**

* Maintains read offset
* Maintains write offset
* Stores access mode
* Points to the corresponding inode

Each open file has one file table entry.

---

### 5) UAREA (User Area)

The UAREA structure represents the **process-level file management**.

**Contains:**

* Process name
* UFDT (User File Descriptor Table)

UFDT is an array where index = file descriptor and value = pointer to file table. This simulates how operating systems map file descriptors to file objects.

---

## Diagram of Data Structures Used in the Project

**Logical Relationship :**

* Boot Block → System startup information
* Super Block → Global file system metadata
* DILB (Linked List of Inodes)
* Each inode → One file
* UFDT (Array)
* Index → File Descriptor
* Value → File Table
* File Table → Points to Inode
* Inode → Points to File Data Buffer

~~~text
+-------------+
|  BootBlock  |
+-------------+

+-------------+
| SuperBlock  |
| TotalInodes |
| FreeInodes  |
+-------------+

Inode Linked List (DILB)
+-------+     +-------+     +-------+
| Inode | --> | Inode | --> | Inode |
+-------+     +-------+     +-------+

UAREA
+----------------------------------+
| Process Name                     |
| UFDT[0] -> NULL                  |
| UFDT[1] -> NULL                  |
| UFDT[2] -> NULL                  |
| UFDT[3] -> FileTable ---> Inode  |
| UFDT[4] -> FileTable ---> Inode  |
+----------------------------------+

~~~

This layered design closely resembles real operating system file management.

---

## Flow of the Project

1. System(Program) starts
2. Boot block initializes
3. Super block initializes
4. DILB (Disk Inode Linked List) is created
5. UAREA and UFDT are initialized
6. CVFS Shell starts and waits for user commands
7. User enters commands
8. Command is parsed
9. Corresponding operation is performed (function is executed)
10. Output is displayed
11. System continues until `exit` command is entered
12. Program terminates

---

## Actual Code of the Project (Functional Overview)

### File Creation

* Checks parameters
* Checks free inode availability
* Allocates inode
* Allocates memory buffer
* Updates super block
* Returns file descriptor

### File Writing

* Validates file descriptor
* Checks write permission
* Checks available space
* Writes data into buffer
* Updates offsets and file size

### File Reading

* Validates file descriptor
* Checks read permission
* Checks available data
* Reads data from buffer
* Updates read offset

### File Deletion

* Locates file
* Frees data buffer
* Resets inode
* Frees file table entry
* Updates super block

### File Listing

* Traverses inode linked list
* Displays active files

---

## Output Demonstration

The output demonstrates:

* Successful system boot
* File creation with file descriptor
* Writing data into files
* Reading data from files
* Listing all files
* Deleting files
* Error handling (permission denied, file not found, insufficient space)

Each feature is verified independently using shell commands.

---

## Function Documentation Standard

Before every function, a structured header is used containing:

* Function name
* Input parameters
* Return value
* Description
* Purpose in project

Function Name: `InitialiseUAREA`

Input Parameters:
None

Return Value:
Void

Description:
Initializes the User Area (UAREA) structure used in the Customised Virtual File System.
This function assigns a default name to the running process and sets all entries of
the User File Descriptor Table (UFDT) to NULL, indicating that no files are opened
when the system starts.

------------------------------------------------------------

Function Name: `InitialiseSuperBlock`

Input Parameters:
None

Return Value:
Void

Description:
Initializes the SuperBlock of the file system. This function sets the total number
of inodes available in the system and initializes the free inode count. At system
startup, all inodes are free and ready to be allocated for file creation.

------------------------------------------------------------

Function Name: `CreateDILB`

Input Parameters:
None

Return Value:
Void

Description:
Creates the Disk Inode List Block (DILB) as a singly linked list of inode structures.
Each inode represents a potential file in the virtual file system. Initially, all
inodes are marked as free, with no file name, no allocated buffer, and default values
for metadata fields.

------------------------------------------------------------

Function Name: `IsFileExist`

Input Parameters:
char *name

Return Value:
true  - File exists in the file system
false - File does not exist in the file system

Description:
Checks whether a file with the given name already exists in the inode list. This
function traverses the linked list of inodes and compares the provided file name
with existing file names. It helps prevent duplicate file creation.

------------------------------------------------------------

Function Name: `CreateFile`

Input Parameters:
char *name
int permission

Return Value:
File Descriptor on success
Error code on failure

Description:
Creates a new regular file in the Customised Virtual File System. This function
validates input parameters, checks for free inode availability, allocates an inode,
creates a file table entry, assigns permissions, allocates memory for the file data
buffer, updates the super block, and finally returns a file descriptor.

------------------------------------------------------------

Function Name: `WriteFile`

Input Parameters:
int fd
char *data
int size

Return Value:
Number of bytes written on success
Error code on failure

Description:
Writes data into the file associated with the given file descriptor. This function
verifies write permissions, checks available space in the file buffer, copies the
data into the file’s buffer, and updates the write offset and actual file size.

------------------------------------------------------------

Function Name: `ReadFile`

Input Parameters:
int fd
char *data
int size

Return Value:
Number of bytes read on success
Error code on failure

Description:
Reads data from the file associated with the given file descriptor into the provided
buffer. This function checks read permissions, ensures sufficient data is available,
copies data from the file buffer, and updates the read offset.

------------------------------------------------------------

Function Name: `UnlinkFile`

Input Parameters:
char *name

Return Value:
EXECUTE_SUCCESS on successful deletion
Error code on failure

Description:
Deletes the specified file from the virtual file system. This function frees the
allocated file data buffer, resets inode metadata, deallocates the file table entry,
updates the super block, and makes the inode available for future file creation.


This improves readability, maintainability, and professional quality of the project.

---

## Internal Working of System Calls

### 1. open()

The `open` system call is used to open an existing file or create a new file.

**Internal Working:**

* The operating system receives a request from the user program.
* It checks whether the file exists in the file system.
* The permissions of the file are verified.
* An inode corresponding to the file is located.
* A file table entry is created and initialized.
* A file descriptor is returned to the user process.

**Purpose:**
It establishes a connection between the user process and the file.

**Screenshot Description:**
Screenshot shows command execution opening a file successfully and returning a valid file descriptor.

---

### 2. close()

The `close` system call is used to close an opened file.

**Internal Working:**

* The operating system identifies the file descriptor.
* Reference count of the file is decreased.
* File table entry is released.
* If no process is using the file, resources are freed.

**Purpose:**
It releases system resources associated with the file.

**Screenshot Description:**
Screenshot shows file descriptor being closed and resources released.

---

### 3. read()

The `read` system call reads data from a file into a user buffer.

**Internal Working:**

* OS validates file descriptor.
* Read permission is checked.
* Data is copied from kernel buffer to user buffer.
* Read offset is updated.

**Purpose:**
To retrieve data stored in a file.

**Screenshot Description:**
Screenshot displays data read successfully from a file.

---

### 4. write()

The `write` system call writes data from user buffer into a file.

**Internal Working:**

* File descriptor validation.
* Write permission verification.
* Space availability check.
* Data copied from user buffer to kernel buffer.
* Write offset and file size updated.

**Purpose:**
To store data inside a file.

**Screenshot Description:**
Screenshot shows data written successfully into a file.

---

### 5. lseek()

The `lseek` system call changes the current read/write offset of a file.

**Internal Working:**

* OS verifies file descriptor.
* Offset value is calculated based on reference point (START, CURRENT, END).
* File offset is updated.

**Purpose:**
Allows random access within a file.

**Screenshot Description:**
Screenshot demonstrates moving file pointer to a new location.

---

### 6. stat()

The `stat` system call retrieves metadata information of a file.

**Internal Working:**

* OS locates inode of the file.
* Metadata like size, permissions, and timestamps are fetched.
* Information is copied to user space.

**Purpose:**
Provides detailed information about a file.

**Screenshot Description:**
Screenshot shows file statistics such as size and permissions.

---

### 7. chmod()

The `chmod` system call changes access permissions of a file.

**Internal Working:**

* OS validates file ownership.
* Permission bits in inode are modified.
* Updated permissions are stored.

**Purpose:**
Controls access rights of files.

**Screenshot Description:**
Screenshot displays permission change of a file.

---

### 8. unlink()

The `unlink` system call deletes a file from the file system.

**Internal Working:**

* File name is removed from directory structure.
* Reference count is checked.
* If zero, inode and data blocks are freed.

**Purpose:**
To permanently remove a file.

**Screenshot Description:**
Screenshot shows file deletion confirmation.

---

# Explanation of Linux Commands

## 1. ls
The `ls` command is used to display the list of files and directories present in the current working directory.  
It helps the user to know what files and folders are available at a particular location in the file system.

---

## 2. ls -l
The `ls -l` command displays files and directories in long listing format.  
It provides detailed information such as file permissions, number of links, owner name, group name, file size, and last modification date.  
This command is useful to understand file attributes and access rights of files.

---

## 3. ls -a
The `ls -a` command displays all files including hidden files.  
Hidden files are those whose names start with a dot (`.`).  
This command is mainly used to view configuration and system-related files which are normally hidden from the user.

---

## 4. rm
The `rm` command is used to remove or delete files from the file system.  
Once a file is deleted using this command, it cannot be recovered easily.  
Therefore, this command should be used carefully.

---

## 5. cat
The `cat` command is used to display the contents of a file on the terminal.  
It can also be used to create files, copy file content, or concatenate multiple files into a single output.  
This command is commonly used for quick viewing of file data.

---

## 6. cd
The `cd` (change directory) command is used to move from one directory to another directory.  
It allows the user to navigate through the directory structure of the file system.

---

## 7. cp
The `cp` command is used to copy files or directories from one location to another.  
It creates a duplicate copy of the source file without affecting or modifying the original file.

---

## 8. mkdir
The `mkdir` command is used to create a new directory.  
It helps in organizing files and directories in a structured manner.

---

## 9. pwd
The `pwd` (print working directory) command displays the absolute path of the current working directory.  
It helps the user to know the exact location where they are currently working in the file system.

---

## 10. touch
The `touch` command is used to create an empty file.  
If the file already exists, it updates the access time and modification time of the file.

---

## 11. stat
The `stat` command displays detailed information about a file or directory.  
It shows metadata such as file size, inode number, file permissions, and time-related information.

---

## 12. man
The `man` command is used to display the manual page of a command.  
It provides detailed documentation including description, usage, options, and examples related to that command.

---

*<h4>CVFS (Question and Answers)</i></h4>*

## 1. What is meant by a file system?

A file system is a method used by an operating system to organize, store, manage, and retrieve data on storage media.  
In this project, the file system is implemented virtually using data structures such as SuperBlock, Inode, File Table, and UAREA to simulate file creation, reading, writing, and deletion in memory.

---

## 2. Which file systems are used by Linux and Windows operating systems?

- **Linux** commonly uses file systems like:
  - ext4
  - ext3
  - XFS
  - Btrfs

- **Windows** commonly uses file systems like:
  - NTFS
  - FAT32
  - exFAT

---

## 3. What are the parts of the file system?

The main parts of the file system used in this project are:

1. Boot Block  
2. Super Block  
3. Disk Inode List Block (DILB)  
4. Inode  
5. File Table  
6. UAREA (User Area)  

---

## 4. Explain UAREA and its contents.

UAREA represents the user process area. It stores information related to the currently running process and the files opened by that process.

### Contents of UAREA:
- `ProcessName`  
  Stores the name of the currently running process.
- `UFDT[MAXOPENFILES]`  
  User File Descriptor Table, an array of pointers to File Table structures representing open files.

---

## 5. Explain the use of the File Table and its contents.

The File Table stores information about files that are currently opened by the process.

### Contents of File Table:
- `ReadOffset` – Indicates the current read position in the file.
- `WriteOffset` – Indicates the current write position in the file.
- `Mode` – Specifies the mode in which the file is opened (read/write).
- `ptrinode` – Pointer to the corresponding inode of the file.

---

## 6. Explain the use of In-Core Inode Table and its use.

The In-Core Inode Table stores inode information in memory while the file is in use.  
It helps in fast access to file metadata without repeatedly searching the disk inode list.

In this project, the inode linked list acts as the in-core inode structure.

---

## 7. What does inode mean?

An inode (Index Node) is a data structure that stores metadata of a file except its name.  
It uniquely identifies a file and contains information required to access file data.

---

## 8. What are the contents of SuperBlock?

The SuperBlock stores overall information about the file system.

### Contents:
- `TotalInodes` – Total number of inodes available.
- `FreeInodes` – Number of free (unused) inodes.

---

## 9. What are the types of files?

In this project, the following file types are used:

1. Regular File  
2. Special File (reserved for future use)

---

## 10. What are the contents of the inode?

Each inode contains metadata of a file.

### Inode Contents:
- FileName  
- InodeNumber  
- FileSize  
- ActualFileSize  
- FileType  
- ReferenceCount  
- Permission  
- Buffer (data storage)  
- Pointer to next inode  

---

## 11. What is the use of a directory file?

A directory file stores the mapping between file names and their corresponding inode numbers.  
It helps the file system locate files efficiently.

---

## 12. How does the operating system maintain security for files?

File security is maintained using permission bits:
- READ
- WRITE
- EXECUTE

Before performing any operation, the system checks whether the requested permission is allowed for the file.

---

## 13. What happens when a user wants to open the file?

When a user opens a file:
1. The system checks whether the file exists.
2. Permission is verified.
3. A File Table entry is created.
4. The inode reference count is incremented.
5. A file descriptor is returned.

---

## 14. What happens when a user calls lseek system call?

The lseek operation changes the current read or write offset of a file.  
It allows moving the file pointer relative to:
- Start of file
- Current position
- End of file

(In this project, lseek support is defined but reserved for future implementation.)

---

## 15. What is the difference between library function and system call?

| Library Function | System Call |
|-----------------|-------------|
| Runs in user mode | Runs in kernel mode |
| Faster execution | Slower execution |
| Does not access hardware directly | Can access hardware |
| Example: printf | Example: read, write |

---

## 16. What is the use of this project?

This project demonstrates:
- Internal working of a file system
- Implementation of file operations using data structures
- Understanding of system-level concepts like inode, file table, and permissions

---

## 17. What are the difficulties that you faced in this project?

- Managing dynamic memory correctly
- Maintaining file offsets
- Handling error conditions
- Designing inode and file table relationships
- Avoiding memory leaks

---

## 18. Is there any improvement needed in this project?

Yes, future improvements include:
- Implementing lseek functionality
- Adding directory hierarchy
- Supporting persistent storage
- Implementing file close operation
- Improving error handling

---

## Summary

These system calls and commands form the foundation of file handling in operating systems. Understanding their internal working provides clarity on how the CVFS project simulates real OS behavior at a conceptual level.

---

## Conclusion

The Customised Virtual File System project provides deep insight into internal working of operating system file management. By implementing core concepts like inodes, file tables, permissions, and file descriptors, this project bridges the gap between theoretical OS concepts and practical implementation.

---

##  License

This project is licensed under the **MIT License** — feel free to modify and use it for learning or development purposes.
See the [LICENSE](LICENSE) file for details.

---

## Author

**Omkar Sachin Narlawar**  

If you like this project, give it a **Star** on  
[GitHub](https://github.com/OmkarNarlawar26)!

Connect to [Linkedin](https://www.linkedin.com/in/omkar-narlawar-7504a8388/)!

---

## 🔗 Useful Links

- [File System – Wikipedia](https://en.wikipedia.org/wiki/File_system)
- [Inode – Wikipedia](https://en.wikipedia.org/wiki/Inode)
- [File Systems in OS – GeeksforGeeks](https://www.geeksforgeeks.org/file-systems-in-operating-system/)
- [Operating System Basics](https://www.geeksforgeeks.org/introduction-of-operating-system-set-1/)
- [Linux File System – Tutorialspoint](https://www.tutorialspoint.com/operating_system/os_file_system.htm)
- [C Programming Reference](https://en.cppreference.com/w/c)
- [Dynamic Memory Allocation in C](https://www.geeksforgeeks.org/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/)
- [Linked List in C](https://www.geeksforgeeks.org/data-structures/linked-list/)
- [Linux read() System Call](https://linux.die.net/man/2/read)
- [Linux write() System Call](https://linux.die.net/man/2/write)

---
