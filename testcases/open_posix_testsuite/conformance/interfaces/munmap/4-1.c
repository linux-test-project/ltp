/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * If a mapping to be removed was private, any modifications
 * made in this address range shall be discarded.
 *
 * Test Step:
 * 1. mmap a file into memory. Set flag as MAP_PRIVATE;
 * 2. Modify the mapped memory, and call msync to try to synchronize the change with
 * 	  the file;
 * 3. munmap the mapped memory;
 * 4. mmap the same file again into memory;
 * 5. If the modification in step 2 appears in the mapped memory, then fail,
 *    otherwise pass.
 */


#include <pthread.h>
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
#include "posixtest.h"

#define TNAME "munmap/4-1.c"

int main(void)
{
	int rc;

	char tmpfname[256];
	char *data;
	int total_size = 1024;

	void *pa = NULL;
	void *addr = NULL;
	size_t size = total_size;
	int flag;
	int fd;
	off_t off = 0;
	int prot;

	char *ch;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_munmap_4_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	unlink(tmpfname);

	data = malloc(total_size);
	memset(data, 'a', total_size);
	if (write(fd, data, total_size) != total_size) {
		printf(TNAME "Error at write(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	free(data);

	prot = PROT_READ | PROT_WRITE;
	flag = MAP_PRIVATE;
	pa = mmap(addr, size, prot, flag, fd, off);
	if (pa == MAP_FAILED) {
		printf("Test Fail: " TNAME " Error at mmap: %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	ch = pa;
	*ch = 'b';

	/* Flush changes back to the file */

	if ((rc = msync(pa, size, MS_SYNC)) != 0) {
		printf(TNAME " Error at msync(): %s\n", strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	munmap(pa, size);

	/* Mmap again */

	pa = mmap(addr, size, prot, flag, fd, off);
	if (pa == MAP_FAILED) {
		printf("Test Fail: " TNAME " Error at 2nd mmap: %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	ch = pa;
	if (*ch == 'b') {
		printf("Test FAIL\n");
		exit(PTS_FAIL);
	}

	close(fd);
	printf("Write referece is discarded when setting MAP_RPIVATE\n");
	printf("Test PASSED\n");
	return PTS_PASS;
}
