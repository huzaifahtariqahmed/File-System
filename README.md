# A Simple File System - A OS Course Project

In this Project we set out to simulate the workings of a simple file system in any usual Operating System.

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
- Just after the free block list, in the super block, we have the inode table containing the 16 inodes themselves.
- Each inode is 56 bytes in size and contains metadata about the stored files/directories as indicated by the data structure in the accompanying filesystem.c.
- Inodes can contain metadata about a file or a directory. The contents of a directory are the a series of directory entries comprising of dirent structures (see filesystem.c).
- The file or directory names can be a maximum of 8 characters including the NULL character.


--- 

**Contributions**: All of the code in this repository is written by [Huzaifah Tariq Ahmed](https://github.com/huzaifahtariqahmed). 
