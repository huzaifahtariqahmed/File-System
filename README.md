# A Simple File System - A OS Course Project

In this Project I set out to simulate the workings of a simple file system in any usual Operating System.

## Specifications

- The whole disk is 128 KB in size.
- The top most directory is the root directory (/).
- The system can have a maximum of 16 files/directories.
- A file can have a maximum of 8 blocks (no indirect pointers). Each block is 1 KB in size.
- A file/directory name can be of 8 chars max (including NULL char). There can be only one file of a given name in a directory.

## Disk Layout

The disk layout1 is as follows: 

- The 128 blocks are divided into 1 super block and 127 data blocks.
- The superblock contains the 128 byte free block list where each byte contains a boolean value indicating whether that particular block is free or not.
- Just after the free block list, in the super block, you have the inode table containing the 16 inodes themselves.
- Each inode is 56 bytes in size and contains metadata about the stored files/directories as indicated by the data structure in the accompanying filesystem.c.
- Inodes can contain metadata about a file or a directory. The contents of a directory are the a series of directory entries comprising of dirent structures (see filesystem.c).
- The file or directory names can be a maximum of 8 characters including the NULL character.

## File System Commands Implemented

### Create a File

SYNTAX: CR filename size

- This command creates a file titled filename of the given size.
- The filename will be an absolute path.
- If there’s not enough space in the disk, it will output an error saying ”not enough space”
- Otherwise it will create a file of the required size filling the file content with small alphabets [a-z] repeated.
- If a directory in the given path does not exist, it will output an error message saying ”the directory XXX in the given path does not exist” where XXX is the name of the missing directory.
- If a file with a given pathname already exist, it will give an error ”the file already exists”.

### Delete a File

SYNTAX: DL filename

- This command will delete a file titled filename.
- The filename will be an absolute path.
- If a directory in the given path does not exist, it will output an error message saying ”the directory XXX in the given path does not exist” where XXX is the name of the missing directory.
- If a file with a given pathname does not exist, it will give an error ”the file does not exist”.

### Copy a File

SYNTAX: CP srcname dstname

- This command will copy a file titled srcname to a file titled dstname.
- The srcname and dstname will be an absolute paths.
- If there’s not enough space in the disk, it will output an error saying ”not enough space”, otherwise it will create a copy of the source file at the destination.
- If a directory in the given paths does not exist, it will output an error message saying ”the directory XXX in the given path does not exist” where XXX is the name of the missing directory.
- If a file with a given pathname already exist, it will overwrite it.
- If either srcname or dstname is a directory, it will give an error saying ”can’t handle directories”.

### Move a file

SYNTAX: MV srcname dstname

- This command will move a file titled srcname to a file titled dstname.
- The srcname and dstname will be an absolute paths.
- This command will not fail due to space limitations if the source and destination files are of the same size.
- If a directory in the given paths does not exist, it will output an error message saying ”the directory XXX in the given path does not exist” where XXX is the name of the missing directory.
- If a file with a given pathname already exists, it will overwrite it.
- If either srcname or dstname is a directory, it will give an error saying ”can’t handle directories”.

### Create a directory

SYNTAX: CD dirname

- This command will create an empty directory at the path indicated by dirname.
- The dirname will be an absolute path.
- If a directory in the given path does not exist, it will output an error message saying ”the directory XXX in the given path does not exist” where XXX is the name of the missing directory.
- If a directory with the given name with the given path exists, it will give an error message saying ”the directory already exists”.

### Remove a directory

SYNTAX: DD dirname

- This command will remove the directory at the path indicated by dirname.
- The dirname will be an absolute path.
- This is a recursive operation, it should remove everything inside the directory from the file system.
- If a directory in the given path does not exist, it will output an error message saying ”the directory XXX in the given path does not exist” where XXX is the name of the missing directory.
- If a directory with the given name at the given path does not exist, it will give an error message saying ”the directory does not exist”.

### List all files

SYNTAX: LL

- This command will list all the files/directories on the hard disk along with their sizes.
- Each file/directory will be listed on a separate line with a space between the name and the size (in bytes).

## Input

- Program will take a command line argument which will be a file containing the commands to be executed as given in the sampleinput.txt.
- It will read the commands and execute them one by one.
- It is assumed that the input will always be in the correct format.
- After executing every command the program will update the state of the hard disk in a file called ”myfs” in the current directory.
- When the program terminates, this file will contain the snapshot of the hard disk at the end of the program.
- At the start of the program, it will look for a file titled ”myfs” in the current directory and will be able to read the hard disk state (if it was stored by the program). If it does not find the file titled ”myfs” in the current directory it will create an empty hard disk by formatting it according to the specified layout and creating the first root directory (/).





--- 

**Contributions**: All of the code in this repository is written by [Huzaifah Tariq Ahmed](https://github.com/huzaifahtariqahmed). 
