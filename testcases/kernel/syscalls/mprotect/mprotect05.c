// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Liam R. Howlett <liam.howlett@oracle.com>
 */

/*\
 * Testcase to check the mprotect(2) system call split and merge.
 *
 * https://bugzilla.kernel.org/show_bug.cgi?id=217061
 *
 */

#include "tst_test.h"

#define TEST_FILE "mprotect05-testfile"

static int fd;
static char *addr = MAP_FAILED;
static unsigned long pagesize;
static unsigned long fullsize;

static void setup(void)
{
	pagesize = getpagesize();
	fullsize = 5 * pagesize;
}

static void run(void)
{
	fd = SAFE_OPEN(TEST_FILE, O_RDWR | O_CREAT, 0777);
	addr = SAFE_MMAP(0, fullsize, PROT_READ, MAP_SHARED, fd, 0);

	if (mprotect(addr + pagesize, pagesize, PROT_EXEC))
		tst_res(TFAIL | TERRNO, "mprotect failed to exec");

	if (mprotect(addr + 3 * pagesize, pagesize, PROT_WRITE))
		tst_res(TFAIL | TERRNO, "mprotect failed to write");

	if (mprotect(addr + pagesize, pagesize * 4, PROT_READ))
		tst_res(TFAIL | TERRNO, "mprotect failed to read");

	SAFE_MUNMAP(addr, fullsize);
	SAFE_CLOSE(fd);
	addr = MAP_FAILED;
	SAFE_UNLINK(TEST_FILE);
	tst_res(TPASS, "test passed");
}

static void cleanup(void)
{
	if (addr != MAP_FAILED) {
		SAFE_MUNMAP(addr, fullsize);
		SAFE_CLOSE(fd);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir  = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "2fcd07b7ccd5"},
		{}
	},
};
