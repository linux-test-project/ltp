// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2023-2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Iterate over all kinds of file descriptors and feed splice() with all possible
 * combinations where at least one file descriptor is invalid. We do expect the
 * syscall to fail either with EINVAL or EBADF.
 */

#define _GNU_SOURCE

#include <sys/socket.h>
#include <netinet/in.h>

#include "tst_test.h"

static void check_splice(struct tst_fd *fd_in, struct tst_fd *fd_out)
{
	/* These combinations just hang since the pipe is empty */
	if (fd_in->type == TST_FD_PIPE_READ) {
		switch (fd_out->type) {
		case TST_FD_FILE:
		case TST_FD_PIPE_WRITE:
		case TST_FD_UNIX_SOCK:
		case TST_FD_INET_SOCK:
		case TST_FD_MEMFD:
			return;
		default:
		break;
		}
	}

	if (fd_out->type == TST_FD_PIPE_WRITE) {
		switch (fd_in->type) {
		/* While these combinations succeeed */
		case TST_FD_DEV_ZERO:
		case TST_FD_FILE:
		case TST_FD_PROC_MAPS:
		case TST_FD_MEMFD:
			return;
		/* And this complains about socket not being connected */
		case TST_FD_INET_SOCK:
			return;
		default:
		break;
		}
	}

	const int exp_errnos[] = {EBADF, EINVAL};

	TST_EXP_FAIL2_ARR(splice(fd_in->fd, NULL, fd_out->fd, NULL, 1, 0),
		exp_errnos, ARRAY_SIZE(exp_errnos), "splice() on %s -> %s",
		tst_fd_desc(fd_in), tst_fd_desc(fd_out));
}

static void verify_splice(void)
{
	TST_FD_FOREACH(fd_in) {
		tst_res(TINFO, "%s -> ...", tst_fd_desc(&fd_in));
		TST_FD_FOREACH(fd_out)
			check_splice(&fd_in, &fd_out);
	}
}

static struct tst_test test = {
	.test_all = verify_splice,
};
