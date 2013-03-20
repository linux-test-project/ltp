/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that mlockall lock the mapped files pages currently mapped into the
 * address space of the process when MCL_CURRENT is set.
 *
 * This test use msync to check that the page is locked.
 */
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "posixtest.h"

int main(void)
{
	void *page_ptr;
	size_t page_size;
	int result, fd;
	void *foo;
	char filename[] = "/tmp/mlockall_3-7-XXXXXX";

	page_size = sysconf(_SC_PAGESIZE);
	if (errno) {
		perror("An error occurs when calling sysconf()");
		return PTS_UNRESOLVED;
	}

	fd = mkstemp(filename);
	if (fd == -1) {
		perror("An error occurs when calling open()");
		return PTS_UNRESOLVED;
	}
	unlink(filename);

	foo = mmap(NULL, page_size, PROT_READ, MAP_SHARED, fd, 0);
	if (foo == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		return PTS_UNRESOLVED;
	}

	if (mlockall(MCL_CURRENT) == -1) {
		if (errno == EPERM) {
			printf
			    ("You don't have permission to lock your address space.\nTry to rerun this test as root.\n");
		} else {
			perror("An error occurs when calling mlockall()");
		}
		return PTS_UNRESOLVED;
	}

	page_ptr = (void *)((long)foo - ((long)foo % page_size));

	result = msync(page_ptr, page_size, MS_SYNC | MS_INVALIDATE);
	if (result == -1 && errno == EBUSY) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result == 0) {
		printf
		    ("The mapped files pages of the process are not locked.\n");
		return PTS_FAIL;
	}
	perror("Unexpected error");
	return PTS_UNRESOLVED;
}
