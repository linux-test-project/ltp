// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that eventfd2 correctly set FD_CLOEXEC flag on file when
 * EFD_CLOEXEC flag is used.
 */

#include <fcntl.h>
#include <sys/eventfd.h>
#include "tst_test.h"
#include "eventfd2.h"

static void run(void)
{
	int fd, flags;

	fd = eventfd2(1, 0);
	flags = SAFE_FCNTL(fd, F_GETFD);
	TST_EXP_EXPR(!(flags & FD_CLOEXEC), "FD_CLOEXEC is not set");
	SAFE_CLOSE(fd);

	fd = eventfd2(1, EFD_CLOEXEC);
	flags = SAFE_FCNTL(fd, F_GETFD);
	TST_EXP_EXPR((flags & FD_CLOEXEC), "FD_CLOEXEC is set");
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
};
