// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_create(2)` fails with EMFILE when the
 * per-process limit on the number of open file descriptors has been reached.
 */

#include <stdlib.h>
#include <sys/epoll.h>

#include "tst_test.h"
#include "lapi/epoll.h"
#include "lapi/syscalls.h"

#include "epoll_create.h"

static int *fds;
static int nfds;
static long maxfds;

static void setup(void)
{
	int fd;

	variant_info();

	maxfds = SAFE_SYSCONF(_SC_OPEN_MAX);
	fds = SAFE_MALLOC(maxfds * sizeof(int));
	memset(fds, -1, maxfds * sizeof(int));

	fds[0] = SAFE_OPEN("dummy", O_RDWR | O_CREAT, 0700);
	nfds = 1;

	for (int i = 1; i < maxfds; i++) {
		fd = dup(fds[0]);
		if (fd == -1)
			break;
		fds[nfds++] = fd;
	}
}

static void run(void)
{
	TST_EXP_FAIL2(do_epoll_create(1), EMFILE,
		      "epoll_create(1) with exhausted fds");

	if (TST_RET != -1)
		SAFE_CLOSE(TST_RET);
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < nfds; i++) {
		if (fds[i] != -1)
			SAFE_CLOSE(fds[i]);
	}

	free(fds);
}

static struct tst_test test = {
	.test_variants = EPOLL_CREATE_VARIANTS,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
