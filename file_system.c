#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_BLOCKS 128
#define FILENAME_MAXLEN 8  // including the NULL char

// inode 

typedef struct inode {
  int  dir;  // boolean value. 1 if it's a directory.
  char name[FILENAME_MAXLEN];
  int  size;  // actual file/directory size in bytes.
  int  blockptrs [8];  // direct pointers to blocks containing file's content.
  int  used;  // boolean value. 1 if the entry is in use.
  int  rsvd;  // reserved for future use
} inode;

// directory entry

typedef struct dirent {
  char name[FILENAME_MAXLEN];
  // int  namelen;  // length of entry name
  int  inode;  // this entry inode index
} dirent;

// Function to find a free inode and update the inode_array
int findFreeInode(struct inode inode_array[]);
int findFreeInode(struct inode inode_array[]) {
    // Implement logic to find and return a free inode
    // You might need to update the inode_array to mark the inode as used
    // Return -1 if no free inode is available
    for(int v = 0;v<16;v++){
      if(inode_array[v].used == 0){
        return v;
      }
    }
    return -1;
}

// Function to find a free data block and update the datablock and free_block_list
int findFreeDataBlock(int free_block_list[]);
int findFreeDataBlock(int free_block_list[]) {
    bool status = false;
    int index;
    for(int n=0;n<127;n++){
      if(free_block_list[n] == 0){
        status = true;
        index = n;
        break;
      }
    }
    if(status == true){
      return index;
    }
    else {
      return -1;
    }
}

// Function to save data to a CSV file
void saveToCSV(const char* filename, char* datablock[], struct inode inode_array[], int free_block_list[]);
void saveToCSV(const char* filename, char* datablock[], struct inode inode_array[], int free_block_list[]) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Write datablock data to CSV
    for (int i = 0; i < 127; i++) {
        if (datablock[i] != NULL) {
            fprintf(file, "DATABLOCK,%d,%s\n", i, datablock[i]);
        }
    }

    // Write inode_array data to CSV
    for (int i = 0; i < 16; i++) {
        if (inode_array[i].used) {
            fprintf(file, "INODE,%d,%d,%d,%s,%d\n", i, inode_array[i].used, inode_array[i].size, inode_array[i].name, inode_array[i].dir);
        }
    }

    // Write free_block_list data to CSV
    for (int i = 0; i < 128; i++) {
        fprintf(file, "FREE_BLOCK,%d,%d\n", i, free_block_list[i]);
    }

    fclose(file);
}

// Function to load data from a CSV file
void loadFromCSV(const char* filename, char* datablock[], struct inode inode_array[], int free_block_list[]);
void loadFromCSV(const char* filename, char* datablock[], struct inode inode_array[], int free_block_list[]) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        char type[50];
        int index, value1, value2;
        char str1[100], str2[100];

        if (sscanf(line, "%[^,],%d,%d,%[^,],%d", type, &index, &value1, str1, &value2) == 5) {
            if (strcmp(type, "DATABLOCK") == 0) {
                if (index >= 0 && index < 127) {
                    strcpy(datablock[index], str1);
                }
            }
            else if (strcmp(type, "INODE") == 0) {
                if (index >= 0 && index < 16) {
                    inode_array[index].used = value1;
                    inode_array[index].size = value2;
                    strcpy(inode_array[index].name, str1);
                    inode_array[index].dir = value2;
                }
            }
            else if (strcmp(type, "FREE_BLOCK") == 0) {
                if (index >= 0 && index < 128) {
                    free_block_list[index] = value1;
                }
            }
        }
    }

    fclose(file);
}

// functions

// create file
void create_file(char filename[], int size, char* datablock[], struct inode inode_array[], int free_block_list[]);
void create_file(char filename[], int size, char* datablock[], struct inode inode_array[], int free_block_list[]){
  char path_separated[100][100];
  int index = 0;
  int i,j = 0;
  for (i = 0; i < strlen(filename); i++) {
    if (filename[i] != '/') {
      if(filename[i] == '\n'){
        continue;
      }
      else{
        path_separated[index][j++] = filename[i];
      }
    } 
    else {
      path_separated[index][j] = '\0'; // Null-terminate the current segment
      index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  path_separated[index][j] = '\0';

  // Traverse the path segments
  int current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  for (int t = 0; t < index; t++) {
    bool dir_or_file_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while(inode_array[current_inode_index].blockptrs[g] != -1){
      int current_data_block_index = inode_array[current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;
      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);

        if (read_offset == 2 && strcmp(current_dir_entry.name, path_separated[t+1]) == 0) {
            // Directory with the same name found, update the current inode and set the flag
            current_inode_index = current_dir_entry.inode;
            dir_or_file_found = true;
            break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if(dir_or_file_found==true){
        break;
      }
      g++;
      
    }

    if (!dir_or_file_found && t!=(index-1)) {
      printf("The directory %s in the given path does not exist%s\n", path_separated[t+1], "");
      break;
    }
    else if(dir_or_file_found && t==(index-1)){
      printf("The file already exists");
      break;
    }
    else if (!dir_or_file_found && t==(index-1)){
      // File not found, create a new one
      int total_blocks = (size + 1024 - 1) / 1024;
      if(total_blocks > 8){
        printf("Exceeding the file size limit\n");
        return;
      }
      int new_inode_index = findFreeInode(inode_array);
      int new_data_block_index;

      // Update the inode_array for the new directory
      inode_array[new_inode_index].dir = 0;
      for(int b=0;b<strlen(path_separated[t+1]);b++){
        inode_array[new_inode_index].name[b] = path_separated[t+1][b];
      }
      inode_array[new_inode_index].name[7] = '\0';
      inode_array[new_inode_index].size = size;
      inode_array[new_inode_index].rsvd = 0;
      inode_array[new_inode_index].used = 1;
      for(int f=0;f<total_blocks;f++){
        new_data_block_index = findFreeDataBlock(free_block_list);
        if (new_inode_index == -1 || new_data_block_index == -1) {
          // Handle error: No free inode or data block available
          printf("Not enough Space");
          return;
        }
        inode_array[new_inode_index].blockptrs[f] = new_data_block_index;
        free_block_list[new_data_block_index] = 1;
      }
      int rem = 8 - total_blocks;
      for(int d=0;d<rem;d++){
        inode_array[new_inode_index].blockptrs[d + total_blocks] = -1;        
      }

      // datablock[inode_array[new_inode_index].blockptrs[0]] = "welcome to new file\n";

      for(int h=0;h<total_blocks;h++){
        if(h!=(total_blocks-1)){
          int char_index = 0;
          for (char ch = 'a'; char_index < 1024; ch++) {
            if (ch > 'z') {
              ch = 'a'; // Reset to 'a' when 'z' is reached
            }
            datablock[inode_array[new_inode_index].blockptrs[h]][char_index] = ch; // Fill the array with 'a' to 'z' repeatedly
            char_index++; // Move to the next index
          }
        }
        else {
          int curr_size = size - (h*1024);
          int char_index = 0;
          for (char ch = 'a'; char_index < curr_size; ch++) {
            if (ch > 'z') {
              ch = 'a'; // Reset to 'a' when 'z' is reached
            }
            datablock[inode_array[new_inode_index].blockptrs[h]][char_index] = ch; // Fill the array with 'a' to 'z' repeatedly
            char_index++; // Move to the next index
          }
        }
      }

      char new_inode_index_str[3]; // Assuming 3 characters are enough for the integer representation
      sprintf(new_inode_index_str, "%d", new_inode_index);
      size_t length = strlen(path_separated[t+1]);

      // Update the current directory's data block with the new directory's entry (you need to implement this)
      int current_db_index = inode_array[current_inode_index].blockptrs[g-1];
      if((1024 - (sizeof(datablock[current_db_index]))) < 25){
        int new_db_index_current_dir = findFreeDataBlock(free_block_list);
        inode_array[current_inode_index].blockptrs[g] = new_db_index_current_dir;
        strncat(datablock[new_db_index_current_dir], path_separated[t+1],length); // Concatenate path_separated[t+1]
        strcat(datablock[new_db_index_current_dir], ","); // Concatenate a comma ","
        strcat(datablock[new_db_index_current_dir],new_inode_index_str);
        strcat(datablock[new_db_index_current_dir], "\n"); // Concatenate a newline "\n"
        inode_array[current_inode_index].size = inode_array[current_inode_index].size + sizeof(path_separated[t+1]) + 3;
      }
      else{
        strncat(datablock[current_db_index],path_separated[t+1],length);
        // printf("%s",datablock[current_db_index]);
        strcat(datablock[current_db_index],",");
        strcat(datablock[current_db_index],new_inode_index_str);
        strcat(datablock[current_db_index],"\n");
        // printf("%s",datablock[current_db_index]);
        inode_array[current_inode_index].size = inode_array[current_inode_index].size + sizeof(path_separated[t+1]) + 3;
      }
      total_blocks = 0;
      rem = 0;
    }
    
  }
  for (int i = 0; i < 127; i++) {
    if (datablock[i] != NULL) { // Check if the data block is allocated
        printf("Datablock %d: %s\n", i, datablock[i]);
    }
  }
  for (int i = 0; i < 16; i++) {
    printf("Entry %d: Inode=%d\n", i, inode_array[i].used);
  }
}

// copy file
void copy_file(char srcname[], char dstname[], char* datablock[], struct inode inode_array[], int free_block_list[]);
void copy_file(char srcname[], char dstname[], char* datablock[], struct inode inode_array[], int free_block_list[]){
  // Split the source and destination paths into segments
  char src_separated[100][100];
  char dst_separated[100][100];
  int src_index = 0, dst_index = 0;
  int i, j = 0;
  
  for (i = 0; i < strlen(srcname); i++) {
    if (srcname[i] != '/') {
      if (srcname[i] == '\n') {
        continue;
      } else {
        src_separated[src_index][j++] = srcname[i];
      }
    } else {
      src_separated[src_index][j] = '\0'; // Null-terminate the current segment
      src_index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  src_separated[src_index][j] = '\0';

  j = 0;
  for (i = 0; i < strlen(dstname); i++) {
    if (dstname[i] != '/') {
      if (dstname[i] == '\n') {
        continue;
      } else {
        dst_separated[dst_index][j++] = dstname[i];
      }
    } else {
      dst_separated[dst_index][j] = '\0'; // Null-terminate the current segment
      dst_index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  dst_separated[dst_index][j] = '\0';

  // Traverse the source path segments
  int src_current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  for (int t = 0; t < src_index; t++) {
    bool src_dir_or_file_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while (inode_array[src_current_inode_index].blockptrs[g] != -1) {
      int current_data_block_index = inode_array[src_current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;
      
      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);

        if (read_offset == 2 && strcmp(current_dir_entry.name, src_separated[t + 1]) == 0) {
          // Directory or file with the same name found, update the current inode and set the flag
          src_current_inode_index = current_dir_entry.inode;
          src_dir_or_file_found = true;
          break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if (src_dir_or_file_found == true) {
        break;
      }
      g++;
    }

    if (!src_dir_or_file_found && t!=(src_index-1)) {
      printf("The directory %s in the source path does not exist\n", src_separated[t + 1]);
      return;
    }
    // Check if srcname is a directory
    else if (src_dir_or_file_found && t==(src_index-1)) {
      if(inode_array[src_current_inode_index].dir == 1){
        printf("Can't handle directories\n");
        return;
      }
    }
  }


  // Traverse the destination path segments
  int dst_current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  for (int t = 0; t < dst_index; t++) {
    bool dst_dir_or_file_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while (inode_array[dst_current_inode_index].blockptrs[g] != -1) {
      int current_data_block_index = inode_array[dst_current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;

      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);

        if (read_offset == 2 && strcmp(current_dir_entry.name, dst_separated[t + 1]) == 0) {
          // Directory or file with the same name found, update the current inode and set the flag
          dst_current_inode_index = current_dir_entry.inode;
          dst_dir_or_file_found = true;
          break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if (dst_dir_or_file_found == true) {
        break;
      }
      g++;
    }

    if (!dst_dir_or_file_found && t != (dst_index - 1)) {
      printf("The directory %s in the destination path does not exist\n", dst_separated[t + 1]);
      return;
    }
    else if (dst_dir_or_file_found && t == (dst_index - 1)) {
      if (inode_array[dst_current_inode_index].dir == 1) {
        printf("Can't handle directories\n");
        return;
      }
      else {
        int total_blocks = (inode_array[src_current_inode_index].size + 1024 - 1) / 1024;
        if (total_blocks > 8) {
          printf("Exceeding the file size limit\n");
          return;
        }
        int new_inode_index = dst_current_inode_index; // Reuse the destination inode
        int new_data_block_index;
        inode_array[new_inode_index].size = inode_array[src_current_inode_index].size;
        for (int b = 0; b < strlen(dst_separated[dst_index]); b++) {
          inode_array[new_inode_index].name[b] = dst_separated[dst_index][b];
        }
        inode_array[new_inode_index].used = 1;
        for (int f = 0; f < total_blocks; f++) {
          new_data_block_index = findFreeDataBlock(free_block_list);
          if (new_inode_index == -1 || new_data_block_index == -1) {
            // Handle error: No free inode or data block available
            printf("Not enough Space\n");
            return;
          }
          inode_array[new_inode_index].blockptrs[f] = new_data_block_index;
          free_block_list[new_data_block_index] = 1;
          memcpy(datablock[new_data_block_index], datablock[inode_array[src_current_inode_index].blockptrs[f]], 1024);
        }
      }
    }
    else if(!dst_dir_or_file_found && t == (dst_index - 1)){
      // File not already there, creating a new one.
      int total_blocks = (inode_array[src_current_inode_index].size + 1024 - 1) / 1024;
      if(total_blocks > 8){
        printf("Exceeding the file size limit\n");
        return;
      }
      int new_inode_index = findFreeInode(inode_array);
      int new_data_block_index;

      // Update the inode_array for the new file
      inode_array[new_inode_index].dir = 0;
      for(int b=0;b<strlen(dst_separated[t+1]);b++){
        inode_array[new_inode_index].name[b] = dst_separated[t+1][b];
      }
      inode_array[new_inode_index].size = inode_array[src_current_inode_index].size;
      inode_array[new_inode_index].rsvd = 0;
      inode_array[new_inode_index].used = 1;
      for(int f=0;f<total_blocks;f++){
        new_data_block_index = findFreeDataBlock(free_block_list);
        if (new_inode_index == -1 || new_data_block_index == -1) {
          // Handle error: No free inode or data block available
          printf("Not enough Space");
          return;
        }
        inode_array[new_inode_index].blockptrs[f] = new_data_block_index;
        free_block_list[new_data_block_index] = 1;
      }
      int rem = 8 - total_blocks;
      for(int d=0;d<rem;d++){
        inode_array[new_inode_index].blockptrs[d + total_blocks] = -1;        
      }

      for(int h=0;h<total_blocks;h++){
        if(h!=(total_blocks-1)){
          int char_index = 0;
          for (char ch = 'a'; char_index < 1024; ch++) {
            if (ch > 'z') {
              ch = 'a'; // Reset to 'a' when 'z' is reached
            }
            datablock[inode_array[new_inode_index].blockptrs[h]][char_index] = ch; // Fill the array with 'a' to 'z' repeatedly
            char_index++; // Move to the next index
          }
        }
        else {
          int curr_size = inode_array[src_current_inode_index].size - (h*1024);
          int char_index = 0;
          for (char ch = 'a'; char_index < curr_size; ch++) {
            if (ch > 'z') {
              ch = 'a'; // Reset to 'a' when 'z' is reached
            }
            datablock[inode_array[new_inode_index].blockptrs[h]][char_index] = ch; // Fill the array with 'a' to 'z' repeatedly
            char_index++; // Move to the next index
          }
        }
      }

      char new_inode_index_str[3]; // Assuming 3 characters are enough for the integer representation
      sprintf(new_inode_index_str, "%d", new_inode_index);
      size_t length = strlen(dst_separated[t+1]);

      // Update the current directory's data block with the new directory's entry (you need to implement this)
      int current_db_index = inode_array[dst_current_inode_index].blockptrs[g-1];
      if((1024 - (sizeof(datablock[current_db_index]))) < 25){
        int new_db_index_current_dir = findFreeDataBlock(free_block_list);
        inode_array[dst_current_inode_index].blockptrs[g] = new_db_index_current_dir;
        strncat(datablock[new_db_index_current_dir], dst_separated[t+1],length); // Concatenate path_separated[t+1]
        strcat(datablock[new_db_index_current_dir], ","); // Concatenate a comma ","
        strcat(datablock[new_db_index_current_dir],new_inode_index_str);
        strcat(datablock[new_db_index_current_dir], "\n"); // Concatenate a newline "\n"
        inode_array[dst_current_inode_index].size = inode_array[dst_current_inode_index].size + sizeof(dst_separated[t+1]) + 3;
      }
      else{
        strncat(datablock[current_db_index],dst_separated[t+1],length);
        // printf("%s",datablock[current_db_index]);
        strcat(datablock[current_db_index],",");
        strcat(datablock[current_db_index],new_inode_index_str);
        strcat(datablock[current_db_index],"\n");
        // printf("%s",datablock[current_db_index]);
        inode_array[dst_current_inode_index].size = inode_array[dst_current_inode_index].size + sizeof(dst_separated[t+1]) + 3;
      }
      total_blocks = 0;
      rem = 0;
    }
  }
  for (int i = 0; i < 127; i++) {
    if (datablock[i] != NULL) { // Check if the data block is allocated
        printf("Datablock %d: %s\n", i, datablock[i]);
    }
  }
  for (int i = 0; i < 16; i++) {
    printf("Entry %d: Inode=%d\n", i, inode_array[i].used);
  }
}

// remove/delete file
void delete_file(char filename[], char* datablock[], struct inode inode_array[], int free_block_list[]);
void delete_file(char filename[], char* datablock[], struct inode inode_array[], int free_block_list[]){
  char path_separated[100][100];
  int index = 0;
  int i,j = 0;
  for (i = 0; i < strlen(filename); i++) {
    if (filename[i] != '/') {
      if(filename[i] == '\n'){
        continue;
      }
      else{
        path_separated[index][j++] = filename[i];
      }
    } 
    else {
      path_separated[index][j] = '\0'; // Null-terminate the current segment
      index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  path_separated[index][j] = '\0';

  int current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  int parent_inode_index = -1; // Parent inode index, initially set to an invalid value
  int last_db_index = -1; // Index of the last data block containing the directory entry

  // Traverse the path segments
  for (int t = 0; t < index; t++) {
    bool dir_or_file_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while (inode_array[current_inode_index].blockptrs[g] != -1) {
      int current_data_block_index = inode_array[current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;

      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);

        if (read_offset == 2 && strcmp(current_dir_entry.name, path_separated[t + 1]) == 0) {
          // Directory or file with the same name found, update the current inode and set the flag
          parent_inode_index = current_inode_index;
          last_db_index = current_data_block_index;
          current_inode_index = current_dir_entry.inode;
          dir_or_file_found = true;
          break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if (dir_or_file_found == true) {
        break;
      }
      g++;
    }

    if (!dir_or_file_found && t != (index - 1)) {
      printf("The directory %s in the given path does not exist\n", path_separated[t + 1]);
      return;
    }
    else if (dir_or_file_found && t == (index - 1)) {
      // Check if the last segment in the path exists and it's a file (not a directory)
      if (!inode_array[current_inode_index].dir) {
        // Mark the associated data blocks as free
        for (int f = 0; f < 8 && inode_array[current_inode_index].blockptrs[f] != -1; f++) {
          int db_index = inode_array[current_inode_index].blockptrs[f];
          free_block_list[db_index] = 0;
          datablock[db_index][0] = '\0'; // Clear the data block content
        }
        for (int f = 0; f < 8; f++) {
          inode_array[current_inode_index].blockptrs[f] = -1;
        }

        // Clear the inode and update the directory entry
        inode_array[current_inode_index].used = 0;
        inode_array[current_inode_index].size = 0;
        inode_array[current_inode_index].dir = 0;
        inode_array[current_inode_index].name[0] = '\0';

        // Remove the directory entry from the parent directory's data block

        char substring[10][100]; // Assuming a maximum substring length of 100 characters
        int len = 0;
        int j = 0;


        for (int i = 0; i < strlen(datablock[last_db_index]); i++) {
            if (datablock[last_db_index][i] != '\n') {
                substring[len][j] = datablock[last_db_index][i];
                j++;
            } else {
                substring[len][j] = '\n'; // Null-terminate the substring
                len++;
                j = 0;
            }
        }
        substring[9][99] = '\0';
      
        datablock[last_db_index][0] = '\0';
        char name[15];
        for(int l = 0;l<len;l++){
          for (int i = 0; i < strlen(substring[l]) && substring[l][i] != ','; i++) {
            if(substring[l][i] != ','){
              name[i] = substring[l][i];
            }
            else{
              name[i] = '\0';
              break;
            }
          }
          if(strcmp(name,path_separated[index]) != 0){
            strncat(datablock[last_db_index],substring[l],20);
            // strcat(datablock[last_db_index],'\n');
          }
        }
        printf("File Deleted Successfully\n");
        break;
      }
      else {
        printf("Can't delete a directory\n");
        break;
      }
    }
    else if(!dir_or_file_found && t==(index-1)){
      printf("the file does not exist\n");
      break;
    }
  }
  for (int i = 0; i < 127; i++) {
    if (datablock[i] != NULL) { // Check if the data block is allocated
        printf("Datablock %d: %s\n", i, datablock[i]);
    }
  }
  for (int i = 0; i < 16; i++) {
    printf("Entry %d: Inode=%d\n", i, inode_array[i].used);
  }
  
}

// move a file
void move_file(char srcname[], char dstname[], char* datablock[], struct inode inode_array[], int free_block_list[]);
void move_file(char srcname[], char dstname[], char* datablock[], struct inode inode_array[], int free_block_list[]){
  // Split the source and destination paths into segments
  char src_separated[100][100];
  char dst_separated[100][100];
  int src_index = 0, dst_index = 0;
  int i, j = 0;
  
  for (i = 0; i < strlen(srcname); i++) {
    if (srcname[i] != '/') {
      if (srcname[i] == '\n') {
        continue;
      } else {
        src_separated[src_index][j++] = srcname[i];
      }
    } else {
      src_separated[src_index][j] = '\0'; // Null-terminate the current segment
      src_index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  src_separated[src_index][j] = '\0';

  j = 0;
  for (i = 0; i < strlen(dstname); i++) {
    if (dstname[i] != '/') {
      if (dstname[i] == '\n') {
        continue;
      } else {
        dst_separated[dst_index][j++] = dstname[i];
      }
    } else {
      dst_separated[dst_index][j] = '\0'; // Null-terminate the current segment
      dst_index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  dst_separated[dst_index][j] = '\0';

  // Traverse the source path segments
  int src_current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  for (int t = 0; t < src_index; t++) {
    bool src_dir_or_file_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while (inode_array[src_current_inode_index].blockptrs[g] != -1) {
      int current_data_block_index = inode_array[src_current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;
      
      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);

        if (read_offset == 2 && strcmp(current_dir_entry.name, src_separated[t + 1]) == 0) {
          // Directory or file with the same name found, update the current inode and set the flag
          src_current_inode_index = current_dir_entry.inode;
          src_dir_or_file_found = true;
          break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if (src_dir_or_file_found == true) {
        break;
      }
      g++;
    }

    if (!src_dir_or_file_found && t!=(src_index-1)) {
      printf("The directory %s in the source path does not exist\n", src_separated[t + 1]);
      return;
    }
    // Check if srcname is a directory
    else if (src_dir_or_file_found && t==(src_index-1)) {
      if(inode_array[src_current_inode_index].dir == 1){
        printf("Can't handle directories\n");
        return;
      }
    }
  }

  // Traverse the destination path segments
  int dst_current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  for (int t = 0; t < dst_index; t++) {
    bool dst_dir_or_file_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while (inode_array[dst_current_inode_index].blockptrs[g] != -1) {
      int current_data_block_index = inode_array[dst_current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;

      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);

        if (read_offset == 2 && strcmp(current_dir_entry.name, dst_separated[t + 1]) == 0) {
          // Directory or file with the same name found, update the current inode and set the flag
          dst_current_inode_index = current_dir_entry.inode;
          dst_dir_or_file_found = true;
          break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if (dst_dir_or_file_found == true) {
        break;
      }
      g++;
    }

    if (!dst_dir_or_file_found && t != (dst_index - 1)) {
      printf("The directory %s in the destination path does not exist\n", dst_separated[t + 1]);
      return;
    }
    else if (dst_dir_or_file_found && t == (dst_index - 1)) {
      if (inode_array[dst_current_inode_index].dir == 1) {
        printf("Can't handle directories\n");
        return;
      }
      else {
        // Delete the source file (similar to your existing code for deleting files)

        // Now, move the source file's data and inode to the destination
        inode_array[dst_current_inode_index].size = inode_array[src_current_inode_index].size;
        for (int b = 0; b < strlen(dst_separated[dst_index]); b++) {
          inode_array[dst_current_inode_index].name[b] = dst_separated[dst_index][b];
        }
        // Update other attributes of the destination inode as needed
        
        // Update data blocks: move data from src to dst (similar to your existing code for copying)
        for (int f = 0; f < 8 && inode_array[src_current_inode_index].blockptrs[f] != -1; f++) {
          // Find a free data block for the destination and copy data
          int new_data_block_index = findFreeDataBlock(free_block_list);
          if (new_data_block_index == -1) {
            // Handle error: No free data block available
            printf("Not enough Space\n");
            return;
          }
          free_block_list[new_data_block_index] = 1;
          memcpy(datablock[new_data_block_index], datablock[inode_array[src_current_inode_index].blockptrs[f]], 1024);
          // Update the destination inode to point to the new data block
          inode_array[dst_current_inode_index].blockptrs[f] = new_data_block_index;
        }

        // Clear the source inode and data blocks
        // You can call your delete_file function here, passing the srcname
        
        delete_file(srcname,datablock,inode_array,free_block_list);

        printf("File moved successfully\n");
        return;
      }
    }
    else if(!dst_dir_or_file_found && t == (dst_index - 1)){
      // File not already there, creating a new one.
      int total_blocks = (inode_array[src_current_inode_index].size + 1024 - 1) / 1024;
      if(total_blocks > 8){
        printf("Exceeding the file size limit\n");
        return;
      }
      int new_inode_index = findFreeInode(inode_array);
      int new_data_block_index;

      // Update the inode_array for the new file
      inode_array[new_inode_index].dir = 0;
      for(int b=0;b<strlen(dst_separated[t+1]);b++){
        inode_array[new_inode_index].name[b] = dst_separated[t+1][b];
      }
      inode_array[new_inode_index].size = inode_array[src_current_inode_index].size;
      inode_array[new_inode_index].rsvd = 0;
      inode_array[new_inode_index].used = 1;
      for(int f=0;f<total_blocks;f++){
        new_data_block_index = findFreeDataBlock(free_block_list);
        if (new_inode_index == -1 || new_data_block_index == -1) {
          // Handle error: No free inode or data block available
          printf("Not enough Space");
          return;
        }
        inode_array[new_inode_index].blockptrs[f] = new_data_block_index;
        free_block_list[new_data_block_index] = 1;
      }
      int rem = 8 - total_blocks;
      for(int d=0;d<rem;d++){
        inode_array[new_inode_index].blockptrs[d + total_blocks] = -1;        
      }

      for(int h=0;h<total_blocks;h++){
        if(h!=(total_blocks-1)){
          int char_index = 0;
          for (char ch = 'a'; char_index < 1024; ch++) {
            if (ch > 'z') {
              ch = 'a'; // Reset to 'a' when 'z' is reached
            }
            datablock[inode_array[new_inode_index].blockptrs[h]][char_index] = ch; // Fill the array with 'a' to 'z' repeatedly
            char_index++; // Move to the next index
          }
        }
        else {
          int curr_size = inode_array[src_current_inode_index].size - (h*1024);
          int char_index = 0;
          for (char ch = 'a'; char_index < curr_size; ch++) {
            if (ch > 'z') {
              ch = 'a'; // Reset to 'a' when 'z' is reached
            }
            datablock[inode_array[new_inode_index].blockptrs[h]][char_index] = ch; // Fill the array with 'a' to 'z' repeatedly
            char_index++; // Move to the next index
          }
        }
      }

      char new_inode_index_str[3]; // Assuming 3 characters are enough for the integer representation
      sprintf(new_inode_index_str, "%d", new_inode_index);
      size_t length = strlen(dst_separated[t+1]);

      // Update the current directory's data block with the new directory's entry (you need to implement this)
      int current_db_index = inode_array[dst_current_inode_index].blockptrs[g-1];
      if((1024 - (sizeof(datablock[current_db_index]))) < 25){
        int new_db_index_current_dir = findFreeDataBlock(free_block_list);
        inode_array[dst_current_inode_index].blockptrs[g] = new_db_index_current_dir;
        strncat(datablock[new_db_index_current_dir], dst_separated[t+1],length); // Concatenate path_separated[t+1]
        strcat(datablock[new_db_index_current_dir], ","); // Concatenate a comma ","
        strcat(datablock[new_db_index_current_dir],new_inode_index_str);
        strcat(datablock[new_db_index_current_dir], "\n"); // Concatenate a newline "\n"
        inode_array[dst_current_inode_index].size = inode_array[dst_current_inode_index].size + sizeof(dst_separated[t+1]) + 3;
      }
      else{
        strncat(datablock[current_db_index],dst_separated[t+1],length);
        // printf("%s",datablock[current_db_index]);
        strcat(datablock[current_db_index],",");
        strcat(datablock[current_db_index],new_inode_index_str);
        strcat(datablock[current_db_index],"\n");
        // printf("%s",datablock[current_db_index]);
        inode_array[dst_current_inode_index].size = inode_array[dst_current_inode_index].size + sizeof(dst_separated[t+1]) + 3;
      }
      total_blocks = 0;
      rem = 0;
    }
  }
  for (int i = 0; i < 127; i++) {
    if (datablock[i] != NULL) { // Check if the data block is allocated
        printf("Datablock %d: %s\n", i, datablock[i]);
    }
  }
  for (int i = 0; i < 16; i++) {
    printf("Entry %d: Inode=%d\n", i, inode_array[i].used);
  }
  }

// list file info
void list_hdd(struct inode inode_array[]);
void list_hdd(struct inode inode_array[]){
  printf("Listing Files and Directories:\n");

    for (int i = 0; i < 16; i++) { // Assuming a maximum of 16 inodes
      if (inode_array[i].used) { // Check if the inode is in use
        if (inode_array[i].dir) {
          printf("Directory: %s\n", inode_array[i].name);
        } else {
          printf("File: %s, Size: %d bytes\n", inode_array[i].name, inode_array[i].size);
        }
      }
    }
    
}
// create directory
void create_dir(char dirname[],char* datablock[],struct inode inode_array[],int free_block_list[]);
void create_dir(char dirname[],char* datablock[],struct inode inode_array[],int free_block_list[]){
  char path_separated[100][100];
  int index = 0;
  int i,j = 0;
  for (i = 0; i < strlen(dirname); i++) {
    if (dirname[i] != '/') {
      if(dirname[i] == '\n'){
        continue;
      }
      else{
        path_separated[index][j++] = dirname[i]; 
      }
    } 
    else {
      path_separated[index][j] = '\0'; // Null-terminate the current segment
      index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  path_separated[index][j] = '\0';

  // Traverse the path segments
  int current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  for (int t = 0; t < index; t++) {
    bool directory_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while(inode_array[current_inode_index].blockptrs[g] != -1){
      int current_data_block_index = inode_array[current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;
      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);  

        if (read_offset == 2 && strcmp(current_dir_entry.name, path_separated[t+1]) == 0) {
            // Directory with the same name found, update the current inode and set the flag
            current_inode_index = current_dir_entry.inode;
            directory_found = true;
            printf("%d\n", current_inode_index);
            break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if(directory_found==true){
        break;
      }
      g++;
      
    }

    if (!directory_found && t!=(index-1)) {
      printf("The directory %s in the given path does not exist%s\n", path_separated[t+1], "");
      break;
    }
    else if ((!directory_found && t==(index-1))){
      // Directory not found, create a new one
      int new_inode_index = findFreeInode(inode_array);
      int new_data_block_index = findFreeDataBlock(free_block_list);

      if (new_inode_index == -1 || new_data_block_index == -1) {
          // Handle error: No free inode or data block available
          printf("Not enough Space");
          return;
      }

      // Update the inode_array for the new directory
      inode_array[new_inode_index].dir = 1;
      for(int b=0;b<strlen(path_separated[t+1]);b++){
        inode_array[new_inode_index].name[b] = path_separated[t+1][b];
      }
      inode_array[new_inode_index].name[7] = '\0';
      inode_array[new_inode_index].size = 0;
      inode_array[new_inode_index].rsvd = 0;
      inode_array[new_inode_index].used = 1;
      inode_array[new_inode_index].blockptrs[0] = new_data_block_index;
      inode_array[new_inode_index].blockptrs[1] = -1;
      inode_array[new_inode_index].blockptrs[2] = -1;
      inode_array[new_inode_index].blockptrs[3] = -1;
      inode_array[new_inode_index].blockptrs[4] = -1;
      inode_array[new_inode_index].blockptrs[5] = -1;
      inode_array[new_inode_index].blockptrs[6] = -1;
      inode_array[new_inode_index].blockptrs[7] = -1;

      free_block_list[new_data_block_index] = 1;

      char new_inode_index_str[3]; // Assuming 20 characters are enough for the integer representation
      sprintf(new_inode_index_str, "%d", new_inode_index);
      size_t length = strlen(path_separated[t+1]);

      // Update the current directory's data block with the new directory's entry (you need to implement this)
      int current_db_index = inode_array[current_inode_index].blockptrs[g-1];
      if((1024 - (sizeof(datablock[current_db_index]))) < 25){
        int new_db_index_current_dir = findFreeDataBlock(free_block_list);
        inode_array[current_inode_index].blockptrs[g] = new_db_index_current_dir;
        strncat(datablock[new_db_index_current_dir], path_separated[t+1],length); // Concatenate path_separated[t+1]
        strcat(datablock[new_db_index_current_dir], ","); // Concatenate a comma ","
        strcat(datablock[new_db_index_current_dir],new_inode_index_str);
        strcat(datablock[new_db_index_current_dir], "\n"); // Concatenate a newline "\n"
        inode_array[current_inode_index].size = inode_array[current_inode_index].size + sizeof(path_separated[t+1]) + 3;
      }
      else{
        strncat(datablock[current_db_index],path_separated[t+1],length);
        // printf("%s",datablock[current_db_index]);
        strcat(datablock[current_db_index],",");
        strcat(datablock[current_db_index],new_inode_index_str);
        strcat(datablock[current_db_index],"\n");
        // printf("%s",datablock[current_db_index]);
        inode_array[current_inode_index].size = inode_array[current_inode_index].size + sizeof(path_separated[t+1]) + 3;
      }

    }
  }
  for (int i = 0; i < 127; i++) {
    if (datablock[i] != NULL) { // Check if the data block is allocated
        printf("Datablock %d: %s\n", i, datablock[i]);
    }
  }
  for (int i = 0; i < 16; i++) {
    printf("Entry %d: Inode=%d\n", i, inode_array[i].used);
  }
  for (int i = 0; i < 15; i++) {
    printf("Free Block %d: %d\n", i, free_block_list[i]);
  }
}

// remove a directory
void remove_dir(char filename[], char* datablock[], struct inode inode_array[], int free_block_list[]);
void remove_dir(char filename[], char* datablock[], struct inode inode_array[], int free_block_list[]){
  char path_separated[100][100];
  int index = 0;
  int i,j = 0;
  for (i = 0; i < strlen(filename); i++) {
    if (filename[i] != '/') {
      if(filename[i] == '\n'){
        continue;
      }
      else{
        path_separated[index][j++] = filename[i];
      }
    } 
    else {
      path_separated[index][j] = '\0'; // Null-terminate the current segment
      index++; // Move to the next index in the result array
      j = 0; // Reset the character index
    }
  }
  path_separated[index][j] = '\0';

  int current_inode_index = 0; // Start from the root inode (assuming root is at index 0)
  int parent_inode_index = -1; // Parent inode index, initially set to an invalid value
  int last_db_index = -1; // Index of the last data block containing the directory entry

  // Traverse the path segments
  for (int t = 0; t < index; t++) {
    bool dir_or_file_found = false;

    // Traverse the entries in the current directory's data block
    int g = 0;
    while (inode_array[current_inode_index].blockptrs[g] != -1) {
      int current_data_block_index = inode_array[current_inode_index].blockptrs[g];
      char* current_data_block = datablock[current_data_block_index];
      int offset = 0;

      while (offset < strlen(current_data_block)) {
        struct dirent current_dir_entry;
        int read_offset = sscanf(current_data_block + offset, "%10[^,],%d\n", current_dir_entry.name, &current_dir_entry.inode);

        if (read_offset == 2 && strcmp(current_dir_entry.name, path_separated[t + 1]) == 0) {
          // Directory or file with the same name found, update the current inode and set the flag
          parent_inode_index = current_inode_index;
          last_db_index = current_data_block_index;
          current_inode_index = current_dir_entry.inode;
          dir_or_file_found = true;
          break;
        }
        // Move the offset to the next entry
        offset += strlen(current_dir_entry.name) + snprintf(NULL, 0, "%d", current_dir_entry.inode) + 2; // +2 for the comma and newline
      }
      if (dir_or_file_found == true) {
        break;
      }
      g++;
    }

    if (!dir_or_file_found && t != (index - 1)) {
      printf("The directory %s in the given path does not exist\n", path_separated[t + 1]);
      return;
    }
    else if (dir_or_file_found && t == (index - 1)) {
      // Check if the last segment in the path exists and it's a file (not a directory)
      if (inode_array[current_inode_index].dir) {
        // Mark the associated data blocks as free
        for (int f = 0; f < 8 && inode_array[current_inode_index].blockptrs[f] != -1; f++) {
          int db_index = inode_array[current_inode_index].blockptrs[f];
          free_block_list[db_index] = 0;
          datablock[db_index][0] = '\0'; // Clear the data block content
        }
        for (int f = 0; f < 8; f++) {
          inode_array[current_inode_index].blockptrs[f] = -1;
        }

        // Clear the inode and update the directory entry
        inode_array[current_inode_index].used = 0;
        inode_array[current_inode_index].size = 0;
        inode_array[current_inode_index].dir = 0;
        inode_array[current_inode_index].name[0] = '\0';

        // Remove the directory entry from the parent directory's data block

        char substring[10][100]; // Assuming a maximum substring length of 100 characters
        int len = 0;
        int j = 0;


        for (int i = 0; i < strlen(datablock[last_db_index]); i++) {
            if (datablock[last_db_index][i] != '\n') {
                substring[len][j] = datablock[last_db_index][i];
                j++;
            } else {
                substring[len][j] = '\n'; // Null-terminate the substring
                len++;
                j = 0;
            }
        }
        substring[9][99] = '\0';
      
        datablock[last_db_index][0] = '\0';
        char name[15];
        for(int l = 0;l<len;l++){
          for (int i = 0; i < strlen(substring[l]) && substring[l][i] != ','; i++) {
            if(substring[l][i] != ','){
              name[i] = substring[l][i];
            }
            else{
              name[i] = '\0';
              break;
            }
          }
          if(strcmp(name,path_separated[index]) != 0){
            strncat(datablock[last_db_index],substring[l],20);
          }
        }
        printf("Directory Deleted Successfully\n");
        break;
      }
      else {
        printf("Can't delete a file\n");
        break;
      }
    }
    else if(!dir_or_file_found && t==(index-1)){
      printf("the directory does not exist\n");
      break;
    }
  }
  for (int i = 0; i < 127; i++) {
    if (datablock[i] != NULL) { // Check if the data block is allocated
        printf("Datablock %d: %s\n", i, datablock[i]);
    }
  }
  for (int i = 0; i < 16; i++) {
    printf("Entry %d: Inode=%d\n", i, inode_array[i].used);
  }
  
}

// main function 

int main (int argc, char* argv[]) {

  char* datablock[127];
  struct inode inode_array[16];
  int free_block_list[128];

  char filename[] = "myfs.csv"; // Replace with the name of the file you want to check

  // Attempt to open the file for reading
  FILE *my_file_sys = fopen(filename, "r");

  if (my_file_sys) {
    printf("File '%s' exists in the current directory.\n", filename);
    loadFromCSV(filename,datablock,inode_array,free_block_list);
  } 
  else {
    // File does not exist
    printf("File '%s' does not exist in the current directory.\n", filename);
    FILE* my_file_sys = fopen(filename, "w");
    for(int a=0;a<128;a++){
      free_block_list[a] = 0;
    }
    for(int n=0;n<16;n++){
      inode_array[n].used = 0;
    }
    free_block_list[0] = 1;
    inode_array[0].dir = 1;
    inode_array[0].name[0] = '/';
    inode_array[0].rsvd = 0;
    inode_array[0].size = 0;
    inode_array[0].used = 1;

    for (int i = 0; i < 127; i++) {
      datablock[i] = (char*)malloc(sizeof(char) * 1024);
      datablock[i][0] = '\0';
    }
    inode_array[0].blockptrs[0] = 0;
    inode_array[0].blockptrs[1] = -1;
    inode_array[0].blockptrs[2] = -1;
    inode_array[0].blockptrs[3] = -1;
    inode_array[0].blockptrs[4] = -1;
    inode_array[0].blockptrs[5] = -1;
    inode_array[0].blockptrs[6] = -1;
    inode_array[0].blockptrs[7] = -1;
  }

  // Reading our input file and calling relevant functions accordingly.

  // while not EOF
  FILE *file = fopen(argv[1], "r");
  char buffer[256];

  while (fgets(buffer, sizeof(buffer), file) != NULL) { // read command

    // parse command

    char fields[5][256];
    int field_count = 0;
    int field_index = 0;
    for(int i = 0; buffer[i] != '\0'; i++) {
      if(buffer[i] != ' ') {
        fields[field_count][field_index++] = buffer[i];
      } 
      else {
        fields[field_count][field_index] = '\0';
        field_count++;
        field_index = 0;
      }
    }
    fields[field_count][field_index] = '\0';
    field_count++;

    // call appropriate function
    
    char field_1[256];
    char field_2[256];

    for (int i = 0; i < strlen(fields[2]); i++) {
      if(fields[2][i] == '\n'){
        field_2[i] = '\0';
      }
      else{
        field_2[i] = fields[2][i];
      }
    } 
    field_2[255] = '\0';


    // printf("%s", field_2);
    // printf("%s\n",field_1);

    if(fields[0][0] == 'C' && fields[0][1] == 'D'){
      strncat(field_1,fields[1],256);
      create_dir(field_1,datablock,inode_array,free_block_list);
    }

    else if(fields[0][0] == 'C' && fields[0][1] == 'R'){
      strncat(field_1,fields[1],256);
      int size = atoi(field_2);
      create_file(field_1,size, datablock, inode_array, free_block_list);
      // charnum[0] = '\0';
    }

    else if(fields[0][0] == 'C' && fields[0][1] == 'P'){
      strncat(field_1,fields[1],256);
      copy_file(field_1,field_2, datablock, inode_array, free_block_list);
    }

    else if(fields[0][0] == 'D' && fields[0][1] == 'L'){
      strncat(field_1,fields[1],256);
      delete_file(field_1,datablock,inode_array,free_block_list);
    }

    else if(fields[0][0] == 'M' && fields[0][1] == 'V'){
      strncat(field_1,fields[1],256);
      move_file(field_1,field_2, datablock, inode_array, free_block_list);
    }

    else if(fields[0][0] == 'L' && fields[0][1] == 'L'){
      list_hdd(inode_array);
    }

    else {
      strncat(field_1,fields[1],256);
      remove_dir(field_1,datablock,inode_array,free_block_list);
    }
    field_1[0] = '\0';
    field_2[0] = '\0';
    saveToCSV(filename,datablock,inode_array,free_block_list);
  }

  return 0;
}
