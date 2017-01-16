#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

/* Check directory Access */
int check_directory_access(char *directory)
{

	struct stat statbuf;

	printf("Checking %s\n", directory);

	if (stat(directory, &statbuf) == -1) {
		printf("FAIL: %s. Could not obtain directory status\n",
		       directory);
		return 1;
	}

	if (statbuf.st_uid != 0) {
		printf("FAIL: %s. Invalid owner\n", directory);
		return 1;
	}

	if ((statbuf.st_mode & S_IWGRP) || (statbuf.st_mode & S_IWOTH)) {
		printf("FAIL: %s. Invalid write access\n", directory);
		return 1;
	}

	printf("PASS: %s\n", directory);
	return 0;
}

int main(int argc, char *argv[])
{

	if (argc != 2) {
		printf("Please enter target directory");
		return 1;
	}

	return check_directory_access(argv[1]);
}
