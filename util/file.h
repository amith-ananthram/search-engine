#ifndef _FILE_H_
#define _FILE_H_

int fileLength(char* filename);

char* readFile(char* filename);

int directoryExists(char* directory_name);

int getFileList(char* directory_name, struct dirent*** files);

int regularFile(char* file_name);

int numFiles(char* directory_name);

#endif
