// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2023-2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that accept() returns ENOTSOCK or EBADF for non-socket file
 * descriptors. The EBADF is returned in the case that the file descriptor has
 * not a file associated with it, which is for example in the case of O_PATH
 * opened file.
 */

#include <sys/socket.h>
#include <netinet/in.h>

#include "tst_test.h"

void check_accept(struct tst_fd *fd)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = 0,
		.sin_addr = {.s_addr = INADDR_ANY},
	};

	socklen_t size = sizeof(addr);

	int exp_errno = ENOTSOCK;

	switch (fd->type) {
	case TST_FD_UNIX_SOCK:
	case TST_FD_INET_SOCK:
		return;
	/*
	 * With these two we fail even before we get to the do_accept() because
	 * the fd does not have a struct file associated.
	 */
	case TST_FD_OPEN_TREE:
	case TST_FD_PATH:
		exp_errno = EBADF;
	default:
		break;
	}

	TST_EXP_FAIL2(accept(fd->fd, (void*)&addr, &size),
		exp_errno, "accept() on %s", tst_fd_desc(fd));
}

static void verify_accept(void)
{
	TST_FD_FOREACH(fd)
		check_accept(&fd);
}

static struct tst_test test = {
	.test_all = verify_accept,
};
