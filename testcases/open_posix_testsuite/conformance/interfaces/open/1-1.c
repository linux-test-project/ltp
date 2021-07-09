/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * Establish the connection between file and file descriptor
 *
 * method:
 *	-open file in write mode using open()
 *	-write 1 sample string into a file
 *	-close the file
 *	-open the same file in read mode
 *	-read data from the file
 *	-compare the value read with the value written in first step
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "open/1-1.c"
#define STR_CONST "POSIX CONFORMANCE TEST"

static void remove_file(char *file_name)
{
	errno = 0;
	(void)unlink(file_name);
}

int main(void)
{
	int test_fd, read_bytes, write_bytes;
	char read_value[sizeof(STR_CONST)] = {};
	char file_str[BUF_SIZE];
	char *filename_p;

	filename_p = tmpnam(file_str);
	if (filename_p == NULL) {
		printf(TNAME " Error in generating tmp file name\n");
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	test_fd = open(filename_p, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXU);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno: %d, %s\n", errno, strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	write_bytes = write(test_fd, STR_CONST, sizeof(STR_CONST));
	if (write_bytes < 0) {
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

	errno = 0;
	read_bytes = read(test_fd, read_value, write_bytes);
	if (read_bytes != write_bytes) {
		printf(TNAME " Wrong data read from file, errno = %d, %s\n", errno, strerror(errno));
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
	if (memcmp(read_value, STR_CONST, read_bytes) == 0) {
		printf(TNAME " Test Passed, string comparison is successful.\n");
		remove_file(filename_p);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Failed, issue in string comparison.\n");
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
}
