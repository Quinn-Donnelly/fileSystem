#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string>
#include "LibFS.h"
#include "LibDisk.h"
#include <sys/stat.h>

// global errno value here
int osErrno;

// Define Helper functions
	// This will check if the given file name currently exists on disk
bool doesExist(const char* fileName);


int FS::FS_Boot(char *path)
{
	printf("FS_Boot %s\n", path);
	std::cout << "Super Block Size = " << sizeof(superBlock) << "\n";
	std::cout << "bitset Size = " << sizeof(std::bitset<1000>) << "\n";
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
			osErrno = E_GENERAL;
			return -1;
		}

		void * sector = new Sector;
		Disk_Read(0, (char*)sector);
		if (((superBlock*)(sector))->magic_num != MAGIC_NUM)
		{
			osErrno = E_GENERAL;
			return -1;
		}
		memcpy(inodeBitmap,((char*)sector + sizeof(superBlock)), sizeof(std::bitset<INODE_NUM>));
		memcpy(dataBitmap, ((char*)sector + sizeof(superBlock) + sizeof(std::bitset<INODE_NUM>)), sizeof(std::bitset<DATA_NUM>));

		delete (Sector*)sector;
	}
	else
	{
		FILE *fp = fopen(path, "ab+");

		if (fp == NULL)
		{
			osErrno = E_GENERAL;
			return -1;
		}

		// Write the superblock into the new disk
		void * buffer = new Sector();
		((superBlock*)buffer)->magic_num = MAGIC_NUM;
		
		memcpy(((char*)buffer + sizeof(superBlock)), inodeBitmap, sizeof(std::bitset<INODE_NUM>));
		memcpy(((char*)buffer + sizeof(superBlock) + sizeof(std::bitset<INODE_NUM>)), dataBitmap, sizeof(std::bitset<DATA_NUM>));

		Disk_Write(0, (char *)buffer);
		Disk_Save(path);

		delete (Sector*)buffer;
		fclose(fp);
	}

	current_path = path;

	return 0;
}

int FS::FS_Sync()
{
	// Saves to disk in the file that the disk was booted from 
	printf("FS_Sync\n");
	Disk_Save(current_path);
	return 0;
}


int FS::File_Create(char *file)
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

int FS::File_Open(char *file)
{
	printf("FS_Open\n");
	return 0;
}

int FS::File_Read(int fd, void *buffer, int size)
{
	printf("FS_Read\n");
	return 0;
}

int FS::File_Write(int fd, void *buffer, int size)
{
	printf("FS_Write\n");
	return 0;
}

int FS::File_Seek(int fd, int offset)
{
	printf("FS_Seek\n");
	return 0;
}

int FS::File_Close(int fd)
{
	printf("FS_Close\n");
	return 0;
}

int FS::File_Unlink(char *file)
{
	printf("FS_Unlink\n");
	return 0;
}


// directory ops
int FS::Dir_Create(char *path)
{
	printf("Dir_Create %s\n", path);
	return 0;
}

int FS::Dir_Size(char *path)
{
	printf("Dir_Size\n");
	return 0;
}

int FS::Dir_Read(char *path, void *buffer, int size)
{
	printf("Dir_Read\n");
	return 0;
}

int FS::Dir_Unlink(char *path)
{
	printf("Dir_Unlink\n");
	return 0;
}



// Define Helper functions
bool doesExist(const char* fileName) {
	struct stat buffer;
	return (stat(fileName, &buffer) == 0);
}
