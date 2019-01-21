/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Implementation performs mapping operations over whole pages.
 * Thus, while the argument len
 * need not meet a size or alignment constraint,
 * the implementation shall include, in any mapping
 * operation, any partial page specified by the range [pa,pa+len).
 * The system shall always zero-fill any partial page at the end of an object.
 * Further, the system shall never write out any modified portions of
 * the last page of an object which are beyond its end.
 *
 * Test Steps:
 * 1. Create a process, in this process:
 *    a. map a file  with size of 1/2 * page_size,
 *       set len = 1/2 * page_size
 *    b. Read the partial page beyond the object size.
 *       Make sure the partial page is zero-filled;
 *    c. Modify a byte in the partial page, then un-map the and close the
 *       file descriptor.
 * 2. Wait for the child proces to exit, then
 *    Map the file again,
 *    read the byte from the position modified at step 1-c and check.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#ifdef	__linux__
#include <sys/vfs.h>
#endif
#include "posixtest.h"

#define TYPE_TMPFS_MAGIC	0x01021994

int main(void)
{
	char tmpfname[256];
	long page_size;

	char *pa;
	size_t len;
	int fd;

	pid_t child;
	int i, exit_val;

	page_size = sysconf(_SC_PAGE_SIZE);

	/* mmap will create a partial page */
	len = page_size / 2;

#ifdef	__linux__
	struct statfs buf;

	if (statfs("/tmp", &buf)) {
		printf("Error at statfs(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	if (buf.f_type == TYPE_TMPFS_MAGIC) {
		printf("From mmap(2) manpage, skip known bug on tmpfs\n");
		return PTS_UNTESTED;
	}
#endif

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_11_5_%d", getpid());
	child = fork();
	switch (child) {
	case 0:
		/* Create shared object */
		unlink(tmpfname);
		fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
			  S_IRUSR | S_IWUSR);
		if (fd == -1) {
			printf("Error at open(): %s\n", strerror(errno));
			return PTS_UNRESOLVED;
		}
		if (ftruncate(fd, len) == -1) {
			printf("Error at ftruncate(): %s\n", strerror(errno));
			return PTS_UNRESOLVED;
		}

		pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (pa == MAP_FAILED) {
			printf("Error at mmap(): %s\n", strerror(errno));
			return PTS_FAIL;
		}

		/* Check the patial page is ZERO filled */
		for (i = len; i < page_size; i++)
			if (pa[i] != 0) {
				printf("Test FAILED: The partial page at the "
				       "end of an object is not zero-filled\n");
				return PTS_FAIL;
			}

		/* Write the partial page */
		pa[len + 1] = 'b';
		msync(pa, len, MS_SYNC);
		munmap(pa, len);
		close(fd);
		return PTS_PASS;
	case -1:
		printf("Error at fork(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	default:
		break;
	}

	wait(&exit_val);
	if (!(WIFEXITED(exit_val) && (WEXITSTATUS(exit_val) == PTS_PASS))) {
		unlink(tmpfname);

		if (WIFEXITED(exit_val))
			return WEXITSTATUS(exit_val);

		printf("Child exited abnormally\n");
		return PTS_UNRESOLVED;
	}

	fd = open(tmpfname, O_RDWR, 0);
	unlink(tmpfname);

	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at 2nd mmap(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	if (pa[len + 1] == 'b') {
		printf("Test FAILED: Modification of the partial page "
		       "at the end of an object is written out\n");
		return PTS_FAIL;
	}
	close(fd);
	munmap(pa, len);

	printf("Test PASSED\n");
	return PTS_PASS;
}
