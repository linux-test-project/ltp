// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef AIODIO_COMMON_H__
#define AIODIO_COMMON_H__

#include <stdlib.h>
#include "tst_test.h"

static inline char *check_zero(char *buf, int size)
{
	char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			tst_res(TINFO,
				"non zero buffer at buf[%lu] => 0x%02x,%02x,%02x,%02x",
				buf - p, (unsigned int)buf[0],
				size > 1 ? (unsigned int)buf[1] : 0,
				size > 2 ? (unsigned int)buf[2] : 0,
				size > 3 ? (unsigned int)buf[3] : 0);
			tst_res(TINFO, "buf %p, p %p", buf, p);
			return buf;
		}
		buf++;
		size--;
	}

	return 0;
}

static inline void io_append(const char *path, char pattern, int flags, size_t bs, size_t bcount)
{
	int fd;
	size_t i;
	char *bufptr;

	bufptr = SAFE_MEMALIGN(getpagesize(), bs);
	memset(bufptr, pattern, bs);

	fd = SAFE_OPEN(path, flags, 0666);

	for (i = 0; i < bcount; i++)
		SAFE_WRITE(1, fd, bufptr, bs);

	free(bufptr);
	SAFE_CLOSE(fd);
}

#endif /* AIODIO_COMMON_H__ */
