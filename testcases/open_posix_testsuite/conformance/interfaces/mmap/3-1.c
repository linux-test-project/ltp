/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mapping established by mmap() shall replace any previous
 * mappings for those whole pages containing any part of the address
 * space of the process starting at pa and continuing for len bytes.
 *
 * Test Steps:
 * 1. Set the size of the file to be mapped as (2 * _SC_PAGE_SIZE);
 * 2. Map size = (_SC_PAGE_SIZE + 2) bytes into memory,
 *    setting the content as 'a'. The mapped address is pa.
 * 2. Map size2 = (_SC_PAGE_SIZE + 1) bytes into memory, starting at the same
 *    address as the first mmap, i.e. pa, using MAP_FIXED flag.
 *    Setting the cotent as 'b'
 * 3. Test whether byte *(pa + size) is 'b'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#ifdef MAP_FIXED
int main(void)
{
	char tmpfname[256];
	char tmpfname2[256];
	char *data;
	long total_size;
	long page_size;

	void *pa;
	size_t size;
	int fd;

	void *pa2;
	size_t size2;
	int fd2;

	char *ch;

	page_size = sysconf(_SC_PAGE_SIZE);
	size = page_size + 2;
	size2 = page_size + 1;

	/* Size of the file */
	total_size = 2 * page_size;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_3_1_%d_1",
		 getpid());
	snprintf(tmpfname2, sizeof(tmpfname2), "/tmp/pts_mmap_3_1_%d_2",
		 getpid());

	unlink(tmpfname);
	unlink(tmpfname2);

	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	fd2 = open(tmpfname2, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1 || fd2 == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	unlink(tmpfname);
	unlink(tmpfname2);

	data = malloc(total_size);

	memset(data, 'a', total_size);
	if (write(fd, data, total_size) != total_size) {
		printf("Error at write(), fd: %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	memset(data, 'b', total_size);
	if (write(fd2, data, total_size) != total_size) {
		printf("Error at write(), fd1: %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	free(data);

	/* Map first file */
	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap: %s\n", strerror(errno));
		return PTS_FAIL;
	}

	ch = pa + size;
	if (*ch != 'a') {
		printf("Test Fail: The file is not mapped correctly\n");
		return PTS_FAIL;
	}

	/* Replace orginal mapping */
	pa2 =
	    mmap(pa, size2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd2,
		 0);
	if (pa2 == MAP_FAILED) {
		printf("Error at 2nd mmap: %s\n", strerror(errno));
		return PTS_FAIL;
	}

	if (pa2 != pa) {
		printf("Error at mmap, the second mmap does not replaced the"
		       " first mapping\n");
		return PTS_FAIL;
	}

	ch = pa2 + size;
	if (*ch != 'b') {
		printf("The original mapped page has not been replaced\n");
		return PTS_FAIL;
	}

	close(fd);
	close(fd2);
	munmap(pa, size);
	munmap(pa2, size2);
	printf("Test PASSED\n");
	return PTS_PASS;
}
#else
int main(void)
{
	printf("MAP_FIXED was not defined at the time of compilation\n");
	return PTS_UNRESOLVED;
}
#endif /* MAP_FIXED */
