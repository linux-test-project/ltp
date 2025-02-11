// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that, mmap() call fails with errno:
 *
 * - EACCES, when a file mapping is requested but the file descriptor is not open for reading.
 * - EINVAL, when length argument is 0.
 * - EINVAL, when flags contains none of MAP_PRIVATE, MAP_SHARED, or MAP_SHARED_VALIDATE.
 */

#include <stdlib.h>
#include "tst_test.h"

#define MMAPSIZE 1024
#define TEMPFILE "mmapfile"
static size_t page_sz;
static int fd;

static struct tcase {
	size_t length;
	int prot;
	int flags;
	int exp_errno;
} tcases[] = {
	{MMAPSIZE, PROT_WRITE, MAP_FILE | MAP_PRIVATE, EACCES},
	{MMAPSIZE, PROT_WRITE, MAP_FILE | MAP_SHARED, EACCES},
	{MMAPSIZE, PROT_READ, MAP_FILE | MAP_PRIVATE, EACCES},
	{MMAPSIZE, PROT_READ, MAP_FILE | MAP_SHARED, EACCES},
	{MMAPSIZE, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, EACCES},
	{MMAPSIZE, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, EACCES},
	{0, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, EINVAL},
	{MMAPSIZE, PROT_READ | PROT_WRITE, MAP_FILE, EINVAL}
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

	TST_EXP_FAIL_PTR_VOID(mmap(NULL, tc->length, tc->prot, tc->flags, fd, 0), tc->exp_errno);

	if (TST_RET_PTR != MAP_FAILED)
		SAFE_MUNMAP(TST_RET_PTR, page_sz);
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
