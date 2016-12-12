#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string>
#include "LibFS.h"
#include "LibDisk.h"

// global errno value here
int osErrno;

// Define Helper functions
	// This will check if the given file name currently exists on disk
bool doesExist(const std::string& fileName);


int
FS_Boot(char *path)
{
	printf("FS_Boot %s\n", path);

	// oops, check for errors
	if (Disk_Init() == -1) {
		printf("Disk_Init() failed\n");
		osErrno = E_GENERAL;
		return -1;
	}

	if (doesExist(path))
	{
		if (Disk_Load(path))
		{
			osErrno = E_CREATE;
		}
	}
	else
	{
		FILE *fp = fopen(path, "ab+");

		if (fp == NULL)
		{
			osErrno = E_CREATE;
		}

		fclose(fp);
	}

	return 0;
}

int
FS_Sync()
{
	printf("FS_Sync\n");
	return 0;
}


int File_Create(char *file)
{
	// Needs to check the file bitmap in the second section to see if the file exists
	printf("FS_Create\n");
	int status = Disk_Save(file);
	if (status == -1)
	{
		std::cout << diskErrno << "\n";
	}
	return 0;
}

int
File_Open(char *file)
{
	printf("FS_Open\n");
	return 0;
}

int
File_Read(int fd, void *buffer, int size)
{
	printf("FS_Read\n");
	return 0;
}

int
File_Write(int fd, void *buffer, int size)
{
	printf("FS_Write\n");
	return 0;
}

int
File_Seek(int fd, int offset)
{
	printf("FS_Seek\n");
	return 0;
}

int
File_Close(int fd)
{
	printf("FS_Close\n");
	return 0;
}

int
File_Unlink(char *file)
{
	printf("FS_Unlink\n");
	return 0;
}


// directory ops
int
Dir_Create(char *path)
{
	printf("Dir_Create %s\n", path);
	return 0;
}

int
Dir_Size(char *path)
{
	printf("Dir_Size\n");
	return 0;
}

int
Dir_Read(char *path, void *buffer, int size)
{
	printf("Dir_Read\n");
	return 0;
}

int
Dir_Unlink(char *path)
{
	printf("Dir_Unlink\n");
	return 0;
}



// Define Helper functions
bool doesExist(const std::string& fileName) {
	struct stat buffer;
	return (stat(fileName.c_str(), &buffer) == 0);
}