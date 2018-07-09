/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * When  O_CREAT flag and O_EXCL flags are set and file already exist, errno will be set
 * to EEXIST
 *
 * method:
 *	-open file using open()
 *	-reopen same file with O_CREAT and O_EXCL flags
 *	-check errno
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
#define TNAME "open/18-1.c"

int main(void)
{
	int test_fd;
	char file_str[BUF_SIZE];
	char *filename_p;
	errno = 0;

	filename_p = tmpnam(file_str);
	if (filename_p == NULL) {
		printf(TNAME " Error in generating tmp file name\n");
		exit(PTS_UNRESOLVED);
	}
	test_fd = open(filename_p, O_CREAT | O_RDWR, S_IRWXG | S_IRWXU);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno: %d, %s\n", errno, strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	if (close(test_fd) < 0) {
		printf(TNAME " Error at close(), errno = %d, %s\n", errno, strerror(errno));
		unlink(filename_p);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	test_fd = open(filename_p, O_CREAT | O_RDWR | O_EXCL);
	if (errno != EEXIST) {
		printf(TNAME " Error, errno is not as expected, errno = %d, %s\n", errno, strerror(errno));
		unlink(filename_p);
		exit(PTS_FAIL);
	} else {
		printf(TNAME " Success, errno is as expected.\n");
		unlink(filename_p);
		exit(PTS_PASS);
	}
}
