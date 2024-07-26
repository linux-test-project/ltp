// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that cachestat() syscall is properly failing with relative
 * error codes according to input parameters.
 *
 * - EFAULT: cstat or cstat_range points to an illegal address
 * - EINVAL: invalid flags
 * - EBADF: invalid file descriptor
 * - EOPNOTSUPP: file descriptor is of a hugetlbfs file
 */

#define MNTPOINT "mnt"

#include "cachestat.h"

static int fd;
static int fd_hugepage;
static int invalid_fd = -1;
static struct cachestat *cs;
static struct cachestat *cs_null;
static struct cachestat_range *cs_range;
static struct cachestat_range *cs_range_null;

static struct tcase {
	int *fd;
	struct cachestat_range **range;
	struct cachestat **data;
	int flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{&invalid_fd, &cs_range, &cs, 0, EBADF, "Invalid fd (-1)"},
	{&fd, &cs_range_null, &cs, 0, EFAULT, "Invalid range (NULL)"},
	{&fd, &cs_range, &cs_null, 0, EFAULT, "Invalid data (NULL)"},
	{&fd, &cs_range, &cs, -1, EINVAL, "Invalid args (-1)"},
	{&fd_hugepage, &cs_range, &cs, 0, EOPNOTSUPP, "Unsupported hugetlbfs"},
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(cachestat(*tc->fd, *tc->range, *tc->data, tc->flags),
		tc->exp_errno, "%s", tc->msg);
}

static void setup(void)
{
	fd = SAFE_OPEN("test", O_CREAT | O_RDWR, 0700);
	fd_hugepage = SAFE_OPEN(MNTPOINT"/test", O_CREAT | O_RDWR, 0700);
}

static void cleanup(void)
{
	SAFE_CLOSE(fd);
	SAFE_CLOSE(fd_hugepage);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.hugepages = {1, TST_NEEDS},
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&cs, .size = sizeof(struct cachestat)},
		{&cs_range, .size = sizeof(struct cachestat_range)},
		{}
	},
};
