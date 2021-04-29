/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the munlock() function sets errno = ENOMEM if some or all of the
 * address range specified by the addr and len arguments does not correspond to
 * valid mapped pages in the address space of the process.
 *
 * Assume that the value LONG_MAX is an invalid pointer.
 *
 * aarch64 linux versions v5.4-rc1 up to v5.6-rc3 may incorrectly report
 * EINVAL instead of ENOMEM, the fix patch see:
 *   597399d0cb91 ("arm64: tags: Preserve tags for addresses translated via TTBR1")
 *   d0022c0ef29b ("arm64: memory: Add missing brackets to untagged_addr() macro")
 *
 * this bug was introduced because of the following commit, see:
 *   057d3389108e ("mm: untag user pointers passed to memory syscalls")
 */

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

#define BUFSIZE 8

int main(void)
{
	int result;
	long page_size;
	void *page_ptr;

	page_size = sysconf(_SC_PAGESIZE);
	if (errno) {
		perror("An error occurs when calling sysconf()");
		return PTS_UNRESOLVED;
	}

	page_ptr = (void *)(LONG_MAX - (LONG_MAX % page_size));
	result = munlock(page_ptr, BUFSIZE);

	if (result == -1 && errno == ENOMEM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		perror("Unexpected error");
		return PTS_UNRESOLVED;
	}

}
