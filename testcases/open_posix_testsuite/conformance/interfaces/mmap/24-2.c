/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [ENOMEM] MAP_FIXED was specified,
 * and the range [addr,addr+len) exceeds that allowed
 * for the address space of a process; or, if MAP_FIXED was not specified and
 * there is insufficient room in the address space to effect the mapping.
 *
 * Test Step:
 * 1. Map a shared memory object, with size exceeding the value get from
 *    rlim_cur of resource RLIMIT_AS, setting MAP_FIXED;
 * 3. Should get ENOMEM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	char tmpfname[256];
	void *pa;
	void *addr;
	size_t len;
	int fd;

	/* Size of the shared memory object */
	size_t shm_size;
	struct rlimit rlim;
	unsigned long page_size = sysconf(_SC_PAGE_SIZE);

	shm_size = 2 * page_size;
	snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_24_2_%d", getpid());

	/* Create shared object */
	shm_unlink(tmpfname);
	fd = shm_open(tmpfname, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at shm_open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	shm_unlink(tmpfname);
	if (ftruncate(fd, shm_size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	/* Currentlly total available memory of this process, in bytes */
	if (getrlimit(RLIMIT_AS, &rlim) == -1) {
		printf("Error at getrlimit(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	printf("available memory: %lu\n", rlim.rlim_cur);

	/* First mmap, just to get a legal addr for second mmap */
	len = shm_size;
	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at first mmap(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	len = rlim.rlim_cur;
	addr = pa;
	printf("addr: %lx, len: %lx\n", (unsigned long)addr,
	       (unsigned long)len);
	/* Make sure addr and len is aligned to page size */
	if ((unsigned long)addr % page_size) {
		/* Upper boundary */
		addr += page_size;
		addr = (void *)((unsigned long)addr & ~(page_size - 1));
	}
	if (len % page_size) {
		/* Lower boundary */
		len &= ~(page_size - 1);
	}
	printf("addr: %lx, len: %lx\n", (unsigned long)addr,
	       (unsigned long)len);
	pa = mmap(addr, len, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd,
		  0);
	if (pa == MAP_FAILED && errno == ENOMEM) {
		printf("Got ENOMEM: %s\nTest PASSED\n", strerror(errno));
		exit(PTS_PASS);
	}

	if (pa == MAP_FAILED)
		perror("Error at mmap()");
	else
		munmap(pa, len);
	close(fd);
	printf("Test Fail: Did not get ENOMEM as expected\n");
	return PTS_FAIL;
}
