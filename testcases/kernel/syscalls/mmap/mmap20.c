// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Paulson Raja L <paulson@zilogic.com>
 */

/*\
 * [Description]
 *
 * Test mmap(2) with MAP_SHARED_VALIDATE flag.
 *
 * Test expected EOPNOTSUPP errno when testing mmap(2) with MAP_SHARED_VALIDATE
 * flag and invalid flag.
 */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include "tst_test.h"
#include "lapi/mmap.h"

#define TEST_FILE "file_to_mmap"
#define TEST_FILE_SIZE 1024
#define INVALID_FLAG (1 << 7)

static int fd = -1;
static void *addr;

static void setup(void)
{
	fd = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR, 0600);

	if (tst_fill_file(TEST_FILE, 'a', TEST_FILE_SIZE, 1))
		tst_brk(TBROK, "Could not fill the testfile");
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);

	if (addr && addr != MAP_FAILED)
		SAFE_MUNMAP(addr, TEST_FILE_SIZE);
}

static void test_mmap(void)
{
	addr = mmap(NULL, TEST_FILE_SIZE, PROT_READ | PROT_WRITE,
			      INVALID_FLAG | MAP_SHARED_VALIDATE, fd, 0);

	if (addr != MAP_FAILED)
		tst_res(TFAIL | TERRNO, "mmap() is successful, but it should have failed");
	else if (errno == EOPNOTSUPP)
		tst_res(TPASS, "mmap() failed with errno set to EOPNOTSUPP");
	else
		tst_res(TFAIL | TERRNO, "mmap() failed with unexpected error");
}

static struct tst_test test = {
	.min_kver = "4.15",
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_mmap,
	.needs_tmpdir = 1,
};
