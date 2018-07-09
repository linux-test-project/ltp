/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * FD_CLOEXEC flag associated with new file descriptor will be cleared,
 * unless O_CLOEXEC oflag is set.
 *
 * method:
 *	-open file in without O_CLOEXEC flag
 *	-use fcntl to get file descriptor flags
 *	-Check if FD_CLOEXEC is not set
 *	-open the same file with O_CLOEXEC flag
 *	-get file descriptor flags and check if FD_CLOEXEC is set
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "open/3-1.c"

static void remove_file(char *file_name)
{
	errno = 0;
	(void)unlink(file_name);
}

int main(void)
{
	int test_fd, flags;
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
	flags = fcntl(test_fd, F_GETFD);
	if (!(flags & FD_CLOEXEC)) {
		remove_file(filename_p);
		printf(TNAME " FD_CLOEXEC flag is cleared by default\n");
	} else {
		printf(TNAME " Error, FD_CLOEXEC flag is not cleared \n");
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
	errno = 0;
	test_fd = open(filename_p, O_CREAT | O_RDWR | O_CLOEXEC);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno = %d, %s\n", errno, strerror(errno));
		remove_file(filename_p);
		exit(PTS_UNRESOLVED);
	}
	flags = fcntl(test_fd, F_GETFD);
	if (flags & FD_CLOEXEC) {
		remove_file(filename_p);
		printf(TNAME " FD_CLOEXEC flag is set when O_CLOEXEC oflag is used.\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Error, FD_CLOEXEC flag is not set \n");
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
}
