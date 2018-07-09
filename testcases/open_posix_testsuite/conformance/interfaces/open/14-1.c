/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * When  O_CREAT flag is set and file does not exist, permission bits for the file which
 * is created shall be set as (mode_t & ~umask).
 *
 * method:
 *	-open file using open()
 *	-check file permissions using stat
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
#define TNAME "open/14-1.c"

int main(void)
{
	int test_fd;
	struct stat sb;
	char file_str[BUF_SIZE];
	errno = 0;
	char *filename_p;
	mode_t old_mask;

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
	stat(filename_p, &sb);
	if (errno != 0) {
		printf(TNAME " Error in getting file stat, errno = %d, %s\n", errno, strerror(errno));
		unlink(filename_p);
		exit(PTS_UNRESOLVED);
	}
	old_mask = umask(0);
	if ((sb.st_mode & ~S_IFMT) == ((S_IRWXG | S_IRWXU) & ~old_mask)) {
		printf(TNAME " Success, permission bits of file is as expected\n");
		umask(old_mask);
		unlink(filename_p);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Error, permission bits of file are not as expected\n");
		umask(old_mask);
		unlink(filename_p);
		exit(PTS_FAIL);
	}
}
