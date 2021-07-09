/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * If a file is opened using open(), the file offset used to mark the current position
 * within the file shall be set to beginning of the file.
 *
 * method:
 *	-open file using open()
 *	-using file descriptor check current position
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "open/4-1.c"
#define STR_CONST "POSIX CONFORMANCE TEST"

static void remove_file(char *file_name)
{
	errno = 0;
	(void)unlink(file_name);
}

int main(void)
{
	int test_fd, ret;
	errno = 0;
	off_t offset;
	char file_str[BUF_SIZE];
	char *filename_p;

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
	ret = write(test_fd, STR_CONST, sizeof(STR_CONST));
	if (ret < 0) {
		printf(TNAME " Error in write()\n");
		remove_file(filename_p);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	if (close(test_fd) < 0) {
		printf(TNAME " Error at close(), errno = %d, %s\n", errno, strerror(errno));
		unlink(filename_p);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	test_fd = open(filename_p, O_RDONLY);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno = %d, %s\n", errno, strerror(errno));
		remove_file(filename_p);
		exit(PTS_UNRESOLVED);
	}
	offset = lseek(test_fd, 0, SEEK_CUR);
	if (offset == 0) {
		printf(TNAME " Test Passed, offset is set to beginning of file\n");
		remove_file(filename_p);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Error, offset is not set to beginning of file\n");
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
}
