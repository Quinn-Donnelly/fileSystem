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
	// s1 is the full path iterator pointing to the current spot and s2 is one path name, determines if it is next in the full path
bool isNextDir(std::string::iterator s1, std::string s2);
	// Returns the length of the char string
int getSize(char name[]);


int FS::FS_Boot(char *path)
{
	printf("FS_Boot %s\n", path);
	// oops, check for errors
	diskAdr = Disk_Init();
	if (diskAdr == NULL) {
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

		// Load the file system sector located at 0th index
		void * sector = new Sector;
		Disk_Read(0, (char*)sector);

		// Check to validate it is our file system
		if (((superBlock*)(sector))->magic_num != MAGIC_NUM)
		{
			osErrno = E_GENERAL;
			return -1;
		}

		// Copy the bitmaps for tracking the current files and directories
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
		
		// Copy in the bisets to follow making sure the root directory is accounted for at inode 0
		((std::bitset<INODE_NUM>*)inodeBitmap)->set(0);
		memcpy(((char*)buffer + sizeof(superBlock)), inodeBitmap, sizeof(std::bitset<INODE_NUM>));
		memcpy(((char*)buffer + sizeof(superBlock) + sizeof(std::bitset<INODE_NUM>)), dataBitmap, sizeof(std::bitset<DATA_NUM>));

		// Save the file system sector
		Disk_Write(0, (char *)buffer);
		Disk_Read(1, (char *)buffer);

		// Make the root directory
		((inode*)buffer)->type = 0;

		// Save the root directory
		Disk_Write(1, (char *)buffer);

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


int FS::File_Create(std::string file)
{
	// Needs to check the file bitmap in the second section to see if the file exists
	printf("FS_Create\n");
	void * dirInode = new Sector;

	// Get the sector with the root inode
	Disk_Read(1, (char *)dirInode);

	int sizeParsed = 0;
	int currentPointer = 0;
	void * dataSector = NULL;

	// Check if the file already is on disk
	while (sizeParsed != ((inode*)dirInode)->size)
	{
		dataSector = ((inode*)dirInode)->dataPtr[currentPointer];
		for (int i = 0; i < 4; ++i)
		{
			if (((dirData*)dataSector)->path_name == file)
			{
				osErrno = E_CREATE;
				return -1;
			}

			sizeParsed += sizeof(dirData);

			if (sizeParsed == ((inode*)dirInode)->size)
			{
				break;
			}
			dataSector = ((dirData*)dataSector) + sizeof(dirData);
		}
		++currentPointer;
	}

	// Find the next open inode if none report error
	int inodeNum;
	bool allocated = false;
	for (int s = 0; s < INODE_NUM; ++s)
	{
		if (((std::bitset<INODE_NUM>*)inodeBitmap)->test(s) == 0)
		{
			inodeNum = s;
			((std::bitset<INODE_NUM>*)inodeBitmap)->set(s);
			allocated = true;
			break;
		}
	}

	if (!allocated)
	{
		osErrno = E_CREATE;
		return -1;
	}

	
	// Write the new file into the dir
	int numEntries = ((inode*)dirInode)->size / sizeof(dirData);
	void * newDirData = ((inode*)dirInode)->dataPtr[numEntries / 4];
	if (newDirData == NULL)
	{
		allocated = false;

		for (int s = 0; s < DATA_NUM; ++s)
		{
			if (((std::bitset<DATA_NUM>*)dataBitmap)->test(s) == 0)
			{
				newDirData = diskAdr + (251 + s);

				((std::bitset<DATA_NUM>*)dataBitmap)->set(s);
				allocated = true;
				break;
			}
		}

		if (!allocated)
		{
			osErrno = E_CREATE;
			return -1;
		}
	}
	newDirData = ((dirData*)newDirData) + numEntries % 4;
	((dirData*)newDirData)->inode_num = inodeNum;

	if (file.size() > 16)
	{
		osErrno = E_CREATE;
		return -1;
	}
	for (int s = 0; s < file.size(); ++s)
	{
		((dirData*)newDirData)->path_name[s] = file[s];
	}

	int sectorNum = inodeNum / 4 + 1;
	// Get the sector with the new inode that has been allocated
	Disk_Read(sectorNum, (char *)dirInode);
	// Add the meta data for the file
	inode * newInode = (inode*)dirInode + inodeNum % 4;
	newInode->size = 0;
	newInode->type = 1;


	delete (Sector*)dirInode;
	return 0;
}

int FS::File_Open(std::string file)
{
	printf("FS_Open\n");

	if (openFiles.size() == 256)
	{
		osErrno = E_TOO_MANY_OPEN_FILES;
		return -1;
	}

	void * dirInode = new Sector;

	// Get the sector with the root inode
	Disk_Read(1, (char *)dirInode);

	int sizeParsed = 0;
	int currentPointer = 0;
	void * dataSector = NULL;

	// Check if the file already is on disk
	while (sizeParsed != ((inode*)dirInode)->size)
	{
		dataSector = ((inode*)dirInode)->dataPtr[currentPointer];
		for (int i = 0; i < 4; ++i)
		{
			if (((dirData*)dataSector)->path_name == file)
			{
				openFiles.insert(std::pair<std::string, int>(file, ((dirData*)dataSector)->inode_num));
				return 0;
			}

			sizeParsed += sizeof(dirData);

			if (sizeParsed == ((inode*)dirInode)->size)
			{
				osErrno = E_NO_SUCH_FILE;
				return -1;
			}

			dataSector = ((dirData*)dataSector) + sizeof(dirData);
		}
		++currentPointer;
	}

	osErrno = E_NO_SUCH_FILE;
	return -1;
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
	char *end = strchr(path, '/');

	// Check if the DIR already exits
	void * buffer = new Sector;
	int status = Dir_Read(path, buffer, SECTOR_SIZE);


	delete (Sector*)(buffer);
	return 0;
}

int FS::Dir_Size(char *path)
{
	printf("Dir_Size\n");
	return 0;
}

int FS::Dir_Read(std::string path, void *buffer, int size)
{
	/*
	printf("Dir_Read\n");
	void * dirInode = new Sector;

	// Get the sector with the root inode
	Disk_Read(1, (char *)dirInode);
	
	int sizeParsed = 0;
	int current_pointer = 0;
	void * entry = NULL;
	std::string pathName;
	std::string::iterator path_it = path.begin() + 1;

	while (sizeParsed != ((inode*)dirInode)->size)
	{
		entry = ((inode*)dirInode)->dataPtr[current_pointer];

		for (int i = 0; i < 4; ++i)
		{
			if (isNextDir(path_it, ((dirData*)entry)->path_name))
			{
				// Get the new dirs inode
				Disk_Read(((dirData*)entry)->inode_num, (char *)dirInode);

				// Adjust the path iterator
				int sizeOfPathName = getSize(((dirData*)entry)->path_name));
				for (int s = 0; s < sizeOfPathName; ++s)
				{
					++path_it;
				}
				// Move past the /
				++path_it;

				//NEEDS TO CHECK IF THIS IS THE LAST ELEMENT IN THE PATH AND RETURN BUFFER

				// Reset sizeParsed
				sizeParsed = 0;
			}

			sizeParsed += sizeof(dirData);
		}

		++current_pointer;
	}

	delete (Sector*)(buffer);
	*/
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

bool isNextDir(std::string::iterator s1, std::string s2)
{
	int spot = 0;

	while (*s1 != '/')
	{
		if (*s1 != s2[spot])
		{
			return false;
		}
		++spot;
		++s1;
	}

	return true;
}

int getSize(char name[])
{
	int size = 0;
	for (int i = 0; i < 16; ++i)
	{
		if (name[i] == '/0')
		{
			return size;
		}
		++size;
	}
}