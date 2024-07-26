// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies cachestat() for all the possible file descriptors,
 * checking that cache statistics are always zero, except for unsupported file
 * descriptors which cause EBADF to be raised.
 */

#include "tst_test.h"
#include "lapi/mman.h"

#define MNTPOINT "mnt"

static struct cachestat *cs;
static struct cachestat_range *cs_range;

static void check_cachestat(struct tst_fd *fd_in)
{
	int ret;

	memset(cs, 0xff, sizeof(*cs));

	ret = cachestat(fd_in->fd, cs_range, cs, 0);
	if (ret == -1) {
		TST_EXP_EQ_LI(errno, EBADF);
		return;
	}

	TST_EXP_EQ_LI(cs->nr_cache, 0);
	TST_EXP_EQ_LI(cs->nr_dirty, 0);
	TST_EXP_EQ_LI(cs->nr_writeback, 0);
	TST_EXP_EQ_LI(cs->nr_evicted, 0);
	TST_EXP_EQ_LI(cs->nr_recently_evicted, 0);
}

static void run(void)
{
	TST_FD_FOREACH(fd) {
		tst_res(TINFO, "%s -> ...", tst_fd_desc(&fd));
		check_cachestat(&fd);
	}
}

static struct tst_test test = {
	.test_all = run,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.bufs = (struct tst_buffers []) {
		{&cs, .size = sizeof(struct cachestat)},
		{&cs_range, .size = sizeof(struct cachestat_range)},
		{}
	},
};

