// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, mmap() call fails when a file mapping is requested but the
 * file descriptor is not open for reading, and errno is set to EACCES.
 */

#include <stdlib.h>
#include "tst_test.h"

#define TEMPFILE "mmapfile"
static size_t page_sz;
static int fd;

static struct tcase {
	int prot;
	int flags;
} tcases[] = {
	{PROT_WRITE, MAP_FILE | MAP_PRIVATE},
	{PROT_WRITE, MAP_FILE | MAP_SHARED},
	{PROT_READ, MAP_FILE | MAP_PRIVATE},
	{PROT_READ, MAP_FILE | MAP_SHARED},
	{PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE},
	{PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED}
};

static void setup(void)
{
	char *buf;

	page_sz = getpagesize();
	buf = SAFE_MALLOC(page_sz);
	memset(buf, 'A', page_sz);

	fd = SAFE_OPEN(TEMPFILE, O_WRONLY | O_CREAT, 0666);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, page_sz);
	free(buf);
}

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TESTPTR(mmap(NULL, page_sz, tc->prot, tc->flags, fd, 0));

	if (TST_RET_PTR != MAP_FAILED) {
		tst_res(TFAIL, "mmap() was successful unexpectedly");
		SAFE_MUNMAP(TST_RET_PTR, page_sz);
	} else if (TST_ERR == EACCES) {
		tst_res(TPASS, "mmap() failed with errno EACCES");
	} else {
		tst_res(TFAIL | TERRNO, "mmap() failed but unexpected errno");
	}
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1
};
