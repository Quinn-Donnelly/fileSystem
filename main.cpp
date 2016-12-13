#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <iostream>
#include "LibFS.h"

#define MAX_PATH_LENGTH 256

using namespace std;

bool checkPath(char *path)
{
	char * it = path;
	int length = 0;
	while (*it != '\0')
	{
		++length;
		++it;
	}

	if (length == 0)
	{
		cout << "Error empty path given\n";
		exit(1);
	}
	else if (!(length <= 256))
	{
		cout << "Error path must be less than " << MAX_PATH_LENGTH << " charecters.\n";
		exit(1);
	}

	return true;
}

void usage(char *prog)
{
	fprintf(stderr, "usage: %s <disk image file>\n", prog);
	exit(1);
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
	}
	char *path = argv[1];
	checkPath(path);

	FS fileSystem;

	fileSystem.FS_Boot(path);
	fileSystem.File_Create("quinn");
	fileSystem.FS_Sync();
	return 0;
}
