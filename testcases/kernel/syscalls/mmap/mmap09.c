// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2003
 * HISTORY
 *	04/2003 Written by Paul Larson
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 *  Verify that truncating a mmaped file works correctly.
 *
 * Expected Result:
 *  ftruncate should be allowed to increase, decrease, or zero the
 *  size of a file that has been mmaped
 *
 *  Test - Use ftruncate to:
 *   1. shrink the file while it is mapped
 *   2. grow the file while it is mapped
 *   3. zero the size of the file while it is mapped
 *
 */

#include "tst_test.h"

/* size of the test file = 64 KB */
#define MAPSIZE (64 * 1024)

static int fd;
static char *maddr;

static struct test_case
{
	off_t newsize;
	char *desc;
} tcases[] = {
	{MAPSIZE - 8192, "ftruncate mmaped file to a smaller size"},
	{MAPSIZE + 1024, "ftruncate mmaped file to a larger size"},
	{0, "ftruncate mmaped file to 0 size"},
};

static void verify_mmap(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	TST_EXP_PASS(ftruncate(fd, tc->newsize), "%s", tc->desc);
}

static void setup(void)
{
	fd = SAFE_OPEN("/tmp/mmaptest", O_RDWR | O_CREAT, 0666);
	/* set the file to initial size */
	SAFE_FTRUNCATE(fd, MAPSIZE);
	maddr = SAFE_MMAP(0, MAPSIZE, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
	/* fill up the file with A's */
	memset(maddr, 'A', MAPSIZE);
}

static void cleanup(void)
{
	if (maddr)
		SAFE_MUNMAP(maddr, MAPSIZE);
	if (fd)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_mmap,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
};
