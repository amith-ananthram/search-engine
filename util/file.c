// Contains various file functions used in crawler and indexer.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <dirent.h>

#include "header.h"
#include "file.h"

// Returns an int correspoding to the size of file filename.
int fileLength(char* filename)
{
	struct stat s;

	stat(filename, &s);
	return s.st_size;
}

// Reads the contents into a malloced string from the file filename and
// returns that string.
char* readFile(char* filename)
{
	FILE *fp;
	char* file_contents;
	
	fp = fopen(filename, "r");

	if(fp == NULL)
	{
		fprintf(stderr, "Open file %s failed.\n", filename);
		exit(-1);
	}

	int file_length = fileLength(filename);

	file_contents = malloc(sizeof(char) * file_length + 1);
	BZERO(file_contents, (file_length + 1));

	if(fread(file_contents, sizeof(char), file_length, fp) < file_length)
		return NULL;

	fclose(fp);

	return file_contents;
}

// Takes a file_name returns 0 if it's a regular file.  1 if not.
int regularFile(char* file_name)
{
	struct stat s;

	return (stat(file_name, &s) == 0 && S_ISREG(s.st_mode));
}

// Checks to see if the directory directory_name exists.  Returns 0 if it does, 1 if not.
int directoryExists(char* directory_name)
{
	struct stat s;
	
	return (stat(directory_name, &s) == 0 && S_ISDIR(s.st_mode));	
}

int numFiles(char* directory_name)
{
	struct stat s;
	
	stat(directory_name, &s);

	return s.st_nlink;
}

// Uses scandir to store all the file names of the files in directory_name into files.
int getFileList(char* directory_name, struct dirent ***files)
{
	int num_files = scandir(directory_name, files, 0, alphasort);

	return num_files;
}
