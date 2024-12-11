// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
 * This Test perform Direct Write/Read from/To hugetlbfs file
 * which is mapped to process address space. The test is checking if it
 * succeeds and data written or read is not corrupted.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"

#define P0 "ffffffff"
#define IOSZ 4096
#define NORMAL_PATH "./"
#define MNTPOINT "hugetlbfs/"

static long hpage_size;
static int fd = -1, nfd = -1;

static void run_test(void)
{
	void *p;
	char buf[IOSZ] __attribute__((aligned(IOSZ)));

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	nfd = tst_creat_unlinked(NORMAL_PATH, O_DIRECT, 0600);
	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	memcpy(p, P0, 8);

	SAFE_WRITE(1, nfd, p, IOSZ);
	SAFE_LSEEK(nfd, 0, SEEK_SET);

	SAFE_READ(1, nfd, buf, IOSZ);
	if (memcmp(P0, buf, 8)) {
		tst_res(TFAIL, "Memory mismatch after Direct-IO write");
		goto cleanup;
	}
	SAFE_LSEEK(nfd, 0, SEEK_SET);

	memset(p, 0, IOSZ);
	SAFE_READ(1, nfd, p, IOSZ);

	if (memcmp(p, P0, 8))
		tst_res(TFAIL, "Memory mismatch after Direct-IO read");
	else
		tst_res(TPASS, "Direct-IO read/write to/from hugepages is successful");
cleanup:
	SAFE_MUNMAP(p, hpage_size);
	SAFE_CLOSE(fd);
	SAFE_CLOSE(nfd);
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SIZE)*1024;
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
	if (nfd > 0)
		SAFE_CLOSE(nfd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {1, TST_NEEDS},
};
