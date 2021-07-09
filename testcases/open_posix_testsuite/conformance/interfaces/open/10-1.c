/*
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 * If a file is opened using open() and O_APPEND flag is set
 * the file offset shall be set to the end of file prior to each write.
 *
 * method:
 *	-open file and write STR_CONST_1
 *	-open file with O_APPEND flag set and write STR_CONST_2
 *	-using lseek set the offset to beginning of the file and write STR_CONST_3
 *	-Check the contents of the file
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "posixtest.h"

#define BUF_SIZE 1024
#define TNAME "open/10-1.c"
#define STR_CONST_1 "POSIX "
#define STR_CONST_2 "CONFORMANCE "
#define STR_CONST_3 "TEST"
#define STR_APPEND "POSIX CONFORMANCE TEST"

static void remove_file(char *file_name)
{
	errno = 0;
	(void)unlink(file_name);
}

int main(void)
{
	int test_fd, ret;
	char result_str[sizeof(STR_APPEND)];
	char file_str[BUF_SIZE];
	char *filename_p;
	errno = 0;

	filename_p = tmpnam(file_str);
	if (filename_p == NULL) {
		printf(TNAME " Error in generating tmp file name\n");
		exit(PTS_UNRESOLVED);
	}
	test_fd = open(filename_p, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXU);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno: %d, %s\n", errno, strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	/* size-1 is required as it considers null byte also */
	ret = write(test_fd, STR_CONST_1, sizeof(STR_CONST_1) - 1);
	if (ret != sizeof(STR_CONST_1) - 1) {
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
	test_fd = open(filename_p, O_WRONLY | O_APPEND);
	if (test_fd < 0) {
		printf(TNAME " Error in open(), errno = %d, %s\n", errno, strerror(errno));
		remove_file(filename_p);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	ret = write(test_fd, STR_CONST_2, sizeof(STR_CONST_2) - 1);
	if (ret != sizeof(STR_CONST_2) - 1) {
		printf(TNAME " Error in write()\n");
		remove_file(filename_p);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	if (lseek(test_fd, 0, SEEK_SET) < 0) {
		printf(TNAME " Error in updating the offset to start of file, errno = %d, %s\n", errno, strerror(errno));
		unlink(filename_p);
		exit(PTS_UNRESOLVED);
	}
	errno = 0;
	ret = write(test_fd, STR_CONST_3, sizeof(STR_CONST_3));
	if (ret != sizeof(STR_CONST_3)) {
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
	ret = read(test_fd, result_str, sizeof(STR_APPEND));
	if (ret != sizeof(STR_APPEND)) {
		printf(TNAME " Error in read()\n");
		remove_file(filename_p);
		exit(PTS_UNRESOLVED);
	}
	if (memcmp(result_str, STR_APPEND, sizeof(STR_APPEND)) == 0) {
		printf(TNAME " Success, file offset is set to end of file for each write with O_APPEND \n");
		remove_file(filename_p);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Error, file offset is not set to end of file with O_APPEND \n");
		remove_file(filename_p);
		exit(PTS_FAIL);
	}
}
