/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The implementation shall require that addr be a multiple
 * of the page size {PAGESIZE}.
 *
 * Test step:
 * Try to call unmap, with addr NOT a multiple of page size.
 * Should get EINVAL.
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

#define TNAME "munmap/3-1.c"

int main(void)
{
	char tmpfname[256];
	long file_size;

	void *pa = NULL;
	void *addr = NULL;
	size_t len;
	int flag;
	int fd;
	off_t off = 0;
	int prot;

	int page_size;

	char *pa2;

	page_size = sysconf(_SC_PAGE_SIZE);
	file_size = 2 * page_size;

	/* We hope to map 2 pages */
	len = page_size + 1;

	/* Create tmp file */
	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_munmap_1_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	unlink(tmpfname);

	if (ftruncate(fd, file_size) == -1) {
		printf("Error at ftruncate: %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	flag = MAP_SHARED;
	prot = PROT_READ | PROT_WRITE;
	pa = mmap(addr, len, prot, flag, fd, off);
	if (pa == MAP_FAILED) {
		printf("Test Unresolved: " TNAME " Error at mmap: %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	/* pa should be a multiple of page size */
	pa2 = pa;

	while (((unsigned long)pa2 % page_size) == 0)
		pa2++;

	close(fd);
	if (munmap(pa2, len) == -1 && errno == EINVAL) {
		printf("Got EINVAL\n");
		printf("Test PASSED\n");
		exit(PTS_PASS);
	} else {
		printf("Test FAILED: " TNAME " munmap returns: %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}
}
