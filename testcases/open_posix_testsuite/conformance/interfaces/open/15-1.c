/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * When  O_CREAT flag and O_DIRECTORY flag are set and directory is not exist
 * new directory shall be created.
 *
 * method:
 *	-open directory using O_CREAT and O_DIRECTORY flags
 *	-check whether directory is created
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "open/15-1.c"

int main(void)
{
	int test_fd, ret;
	struct stat sb;
	char dir_str[BUF_SIZE];
	char *dirname_p;
	errno = 0;

	dirname_p = tmpnam(dir_str);
	if (dirname_p == NULL) {
		printf(TNAME " Error in generating tmp dir name\n");
		exit(PTS_UNRESOLVED);
	}
	test_fd = open(dirname_p, O_CREAT | O_DIRECTORY, S_IRWXG | S_IRWXU);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno: %d, %s\n", errno, strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	if (close(test_fd) < 0) {
		printf(TNAME " Error at close(), errno = %d, %s\n", errno, strerror(errno));
		remove(dirname_p);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	ret = stat(dirname_p, &sb);
	if (ret != 0 || errno != 0) {
		printf(TNAME " Error in getting stat, errno = %d, %s\n", errno, strerror(errno));
		remove(dirname_p);
		exit(PTS_FAIL);
	}
	printf("%d \n", S_ISDIR(sb.st_mode));
	if (S_ISDIR(sb.st_mode)) {
		printf(TNAME " Success, Directory is created \n");
		remove(dirname_p);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Error, Directory is not created \n");
		remove(dirname_p);
		exit(PTS_FAIL);
	}
}
