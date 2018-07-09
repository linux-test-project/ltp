/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * When O_RDONLY flag is used, file is opened for read only.
 *
 * method:
 *	-open file with O_EXEC flag
 *	-check read and write operations on file using file descriptor
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "posixtest.h"
#include <sys/stat.h>

#define BUF_SIZE 1024
#define TNAME "open/6-1.c"

static void remove_file(char *file_name)
{
	errno = 0;
	(void)unlink(file_name);
}

int main(void)
{
	int test_fd;
	int *r_ptr, r_val = 0;
	r_ptr = &r_val;
	char file_str[BUF_SIZE];
	char *filename_p;

	filename_p = tmpnam(file_str);
	if (filename_p == NULL) {
		printf(TNAME " Error in generating tmp file name\n");
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	test_fd = open(filename_p, O_CREAT | O_RDONLY, S_IRWXG | S_IRWXU);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno: %d, %s\n", errno, strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	read(test_fd, r_ptr, sizeof(r_val));
	if (errno == 0)
		printf(TNAME " Read operation is allowed on file\n");
	else {
		printf(TNAME " Error, read operation is not allowed on file. errno: %d, %s\n", errno, strerror(errno));
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
	errno = 0;
	write(test_fd, r_ptr, sizeof(r_val));
	if (errno == EBADF) {
		printf(TNAME " Success, write operation is not allowed on file.\n");
		remove_file(filename_p);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Error, write operation is allowed on file \n");
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
}
