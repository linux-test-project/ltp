// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Author: Rajeev Tiwari: rajeevti@in.ibm.com
 * Copyright (c) 2004 Gernot Payer <gpayer@suse.de>
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 *
 * This test case provides a functional validation for mincore system call.
 * We mmap a file of known size (multiple of page size) and lock it in
 * memory. Then we obtain page location information via mincore and compare
 * the result with the expected value.
 */

#include "tst_test.h"

#define NUM_PAGES 4

static void *addr;
static int size;
static unsigned char vec[NUM_PAGES];
static int fd;

static void cleanup(void)
{
	if (addr) {
		SAFE_MUNLOCK(addr, size);
		SAFE_MUNMAP(addr, size);
	}
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void setup(void)
{
	size = getpagesize() * NUM_PAGES;
	char buf[size];

	memset(buf, 42, size);

	int fd = SAFE_OPEN("mincore02", O_CREAT | O_RDWR, 0600);

	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, size);
	addr = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);

	SAFE_MLOCK(addr, size);
}

static void check_mincore(void)
{
	TST_EXP_PASS(mincore(addr, size, vec));

	int locked_pages = 0;

	for (int i = 0; i < NUM_PAGES; i++)
		locked_pages += (vec[i] & 1);

	TST_EXP_EQ_SZ(locked_pages, NUM_PAGES);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = check_mincore,
	.needs_tmpdir = 1,
};
