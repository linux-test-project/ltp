// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that eventfd2 semaphore-like support is properly working.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include "tst_test.h"
#include "eventfd2.h"

static void xsem_wait(int fd)
{
	u_int64_t cntr;

	SAFE_READ(0, fd, &cntr, sizeof(cntr));
}

static void xsem_post(int fd, int count)
{
	u_int64_t cntr = count;

	SAFE_WRITE(0, fd, &cntr, sizeof(cntr));
}

static void sem_player(int fd1, int fd2)
{
	pid_t pid = getpid();

	tst_res(TINFO, "[%u] posting 1 on fd=%d", pid, fd1);
	xsem_post(fd1, 1);

	tst_res(TINFO, "[%u] waiting on fd=%d", pid, fd2);
	xsem_wait(fd2);

	tst_res(TINFO, "[%u] posting 5 on fd=%d", pid, fd1);
	xsem_post(fd1, 5);

	tst_res(TINFO, "[%u] waiting 5 times on fd=%d", pid, fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);

	tst_res(TPASS, "[%u] received all events", pid);
}

static void run(void)
{
	pid_t cpid_poster, cpid_waiter;
	int fd1, fd2;

	fd1 = eventfd2(0, EFD_SEMAPHORE);
	fd2 = eventfd2(0, EFD_SEMAPHORE);

	cpid_poster = SAFE_FORK();
	if (!cpid_poster) {
		sem_player(fd1, fd2);
		exit(0);
	}

	cpid_waiter = SAFE_FORK();
	if (!cpid_waiter) {
		sem_player(fd2, fd1);
		exit(0);
	}
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
