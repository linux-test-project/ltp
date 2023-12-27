// SPDX-License-Identifier: GPL-2.0

#ifndef MEMCONTROL_COMMON_H__
#define MEMCONTROL_COMMON_H__

#include <stdlib.h>
#include <stdio.h>

#include "tst_test.h"

#define TMPDIR "mntdir"
#define MB(x) (x << 20)

/*
 * Checks if two given values differ by less than err% of their
 * sum. An extra percent is added for every doubling of the page size
 * to compensate for wastage in page sized allocations.
 */
static inline int values_close(const ssize_t a,
			       const ssize_t b,
			       const ssize_t err)
{
	const size_t page_size = SAFE_SYSCONF(_SC_PAGESIZE);
	const ssize_t page_adjusted_err = ffs(page_size >> 13) + err;

	return 100 * labs(a - b) <= (a + b) * page_adjusted_err;
}

static inline void alloc_pagecache(const int fd, size_t size)
{
	char buf[BUFSIZ];
	size_t i;

	SAFE_LSEEK(fd, 0, SEEK_END);

	for (i = 0; i < size; i += sizeof(buf))
		SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, sizeof(buf));
}

static inline void alloc_anon(const size_t size)
{
	const size_t page_size = SAFE_SYSCONF(_SC_PAGESIZE);
	char *const buf = SAFE_MALLOC(size);
	size_t i;

	for (i = 0; i < size; i += page_size)
		buf[i] = 0;

	free(buf);
}

#endif /* MEMCONTROL_COMMON_H__ */
