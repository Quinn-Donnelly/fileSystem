#include <stdio.h>
#include <iostream>
#include "LibFS.h"

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
	else if (length > 15)
	{
		cout << "Error path must be less than 16 charecters.\n";
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

	FS_Boot(path);
	FS_Sync();
	return 0;
}
