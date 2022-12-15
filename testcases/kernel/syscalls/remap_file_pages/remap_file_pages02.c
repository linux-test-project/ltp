// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Ricardo Salveti de Araujo <rsalvetidev@gmail.com>, 2007
 *
 * DESCRIPTION
 *     The remap_file_pages() system call is used to create a non-linear
 *     mapping, that is, a mapping in which the pages of the file are mapped
 *     into a non-sequential order in memory.  The advantage of using
 *     remap_file_pages() over using repeated calls to mmap(2) is that
 *     the former  approach  does  not require the kernel to create
 *     additional VMA (Virtual Memory Area) data structures.
 *
 *     Runs remap_file_pages with wrong values and see if got the expected error
 *
 *     Setup:
 *       1. Global:
 *       2. Create a file, do a normal mmap with MAP_SHARED flag
 *
 *     Test:
 *       1. Test with a valid mmap but without MAP_SHARED flag
 *       2. Test with a invalid start argument
 *       3. Test with a invalid size argument
 *       4. Test with a invalid prot argument
 *
 * HISTORY
 *
 *     02/11/2008 - Removed the pgoff test case, as the latest kernels doesn't
 *     verify the page offset (http://lkml.org/lkml/2007/11/29/325) - Ricardo
 *     Salveti de Araujo, <rsalvetidev@gmail.com>
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

#define WINDOW_START 0x48000000

static unsigned int page_sz;

static int fd;
static char *data = NULL;
static char *data01 = NULL;

static void setup01(int test);
static void setup02(int test);
static void setup03(int test);
static void setup04(int test);

static struct tcase {
	char *err_desc;
	int exp_errno;
	void (*setup)(int);

	void *start;
	size_t size;
	int prot;
	ssize_t pgoff;
	int flags;
} tcases[] = {
	{"start is not valid MAP_SHARED mapping",
	 EINVAL, setup01, NULL, 0, 0, 2, 0},
	{"start is invalid", EINVAL, setup02, NULL, 0, 0, 2, 0},
	{"size is invalid", EINVAL, setup03,  NULL, 0, 0, 0, 0},
	{"prot is invalid", EINVAL, setup04, NULL, 0, 0, 2, 0}
};

static void run(unsigned i)
{
	TEST(tst_syscall(__NR_remap_file_pages,
			 tcases[i].start, tcases[i].size,
			 tcases[i].prot, tcases[i].pgoff,
			 tcases[i].flags));

	if ((TST_RET == -1) && (TST_ERR == tcases[i].exp_errno)) {
		tst_res(TPASS | TTERRNO, "remap_file_pages(2) %s",
			tcases[i].err_desc);
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"remap_file_pages(2) %s expected %s got",
		tcases[i].err_desc, tst_strerrno(tcases[i].exp_errno));
}

static void setup01(int test)
{
	tcases[test].start = data01;
	tcases[test].size = page_sz;
}

static void setup02(int test)
{
	tcases[test].start = data + page_sz;
	tcases[test].size = page_sz;
}

static void setup03(int test)
{
	tcases[test].start = data;
	tcases[test].size = 2 * page_sz;
}

static void setup04(int test)
{
	tcases[test].start = data;
	tcases[test].size = page_sz;
	tcases[test].prot = -1;
}

static void setup(void)
{
	unsigned int i;

	page_sz = getpagesize();

	fd = SAFE_OPEN("cache", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
	SAFE_FTRUNCATE(fd, page_sz);

	data = SAFE_MMAP((void *)WINDOW_START, page_sz, PROT_READ | PROT_WRITE,
			 MAP_SHARED, fd, 0);

	data01 = SAFE_MMAP(NULL, page_sz, PROT_READ | PROT_WRITE, MAP_PRIVATE,
			   fd, 0);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].setup)
			tcases[i].setup(i);
	}
}

static void cleanup(void)
{
	SAFE_CLOSE(fd);

	if (data)
		SAFE_MUNMAP(data, page_sz);

	if (data01)
		SAFE_MUNMAP(data01, page_sz);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.cleanup = cleanup,
	.setup = setup,
	.needs_tmpdir = 1,
};
