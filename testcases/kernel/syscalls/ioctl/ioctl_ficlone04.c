// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that :man2:`ioctl` FICLONE/FICLONERANGE feature raises the
 * right error according with bad file descriptors.
 */

#include "tst_test.h"
#include "lapi/ficlone.h"

static void test_bad_fd(struct tst_fd *fd_src, struct tst_fd *fd_dst)
{
	if (fd_src->type == TST_FD_FILE && fd_src->type == fd_dst->type) {
		tst_res(TINFO, "Skipping file: SUCCESS");
		return;
	}

	int exp_errnos[] = {
		EOPNOTSUPP,
		EPERM,
		EISDIR,
		EBADF,
		EINVAL,
		EXDEV,
	};

	TST_EXP_FAIL2_ARR(ioctl(fd_dst->fd, FICLONE, fd_src->fd),
		exp_errnos, ARRAY_SIZE(exp_errnos),
		"ioctl(%s, FICLONE, %s)",
		tst_fd_desc(fd_src),
		tst_fd_desc(fd_dst));
}

static void run(void)
{
	TST_FD_FOREACH(fd_src) {
		TST_FD_FOREACH(fd_dst)
			test_bad_fd(&fd_src, &fd_dst);
	}
}

static struct tst_test test = {
	.test_all = run,
	.min_kver = "4.5",
	.needs_root = 1,
	.needs_tmpdir = 1,
};
