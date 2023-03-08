// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that eventfd2 correctly set O_NONBLOCK flag on file when
 * EFD_NONBLOCK flag is used.
 */

#include <fcntl.h>
#include <sys/eventfd.h>
#include "tst_test.h"
#include "eventfd2.h"

static void run(void)
{
	int fd, flags;

	fd = eventfd2(1, 0);
	flags = SAFE_FCNTL(fd, F_GETFL);
	TST_EXP_EXPR(!(flags & O_NONBLOCK), "O_NONBLOCK is not set");
	SAFE_CLOSE(fd);

	fd = eventfd2(1, EFD_NONBLOCK);
	flags = SAFE_FCNTL(fd, F_GETFL);
	TST_EXP_EXPR((flags & O_NONBLOCK), "O_NONBLOCK is set");
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
};
