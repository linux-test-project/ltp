// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Ltd.
 */

/*\
 * :man2:`select` failure tests:
 *
 * - negative nfds (EINVAL)
 * - invalid readfds (EBADF)
 * - invalid writefds (EBADF)
 * - invalid exceptfds (EBADF)
 * - faulty readfds (EFAULT)
 * - faulty writefds (EFAULT)
 * - faulty exceptfds (EFAULT)
 * - faulty timeout (EFAULT)
 */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "select_var.h"

static fd_set readfds_reg, writefds_reg, fds_closed;
static fd_set *preadfds_reg = &readfds_reg, *pwritefds_reg = &writefds_reg;
static fd_set *pfds_closed = &fds_closed, *nullfds = NULL, *faulty_fds;
static int fd_closed, fd[2];
static int negative_nfds = -1, maxfds;
static struct timeval timeout = {.tv_sec = 0, .tv_usec = 100000};

static struct timeval *valid_to = &timeout, *invalid_to;

static struct tcases {
	char *name;
	int *nfds;
	fd_set **readfds;
	fd_set **writefds;
	fd_set **exceptfds;
	struct timeval **timeout;
	int exp_errno;
} tests[] = {
	{ "Negative nfds", &negative_nfds, &preadfds_reg, &pwritefds_reg, &nullfds, &valid_to, EINVAL },
	{ "Invalid readfds", &maxfds, &pfds_closed, &pwritefds_reg, &nullfds, &valid_to, EBADF },
	{ "Invalid writefds", &maxfds, &preadfds_reg, &pfds_closed, &nullfds, &valid_to, EBADF },
	{ "Invalid exceptfds", &maxfds, &preadfds_reg, &pwritefds_reg, &pfds_closed, &valid_to, EBADF },
	{ "Faulty readfds", &maxfds, &faulty_fds, &pwritefds_reg, &nullfds, &valid_to, EFAULT },
	{ "Faulty writefds", &maxfds, &preadfds_reg, &faulty_fds, &nullfds, &valid_to, EFAULT },
	{ "Faulty exceptfds", &maxfds, &preadfds_reg, &pwritefds_reg, &faulty_fds, &valid_to, EFAULT },
	{ "Faulty timeout", &maxfds, &preadfds_reg, &pwritefds_reg, &nullfds, &invalid_to, EFAULT },
};

static void verify_select(unsigned int n)
{
	struct tcases *tc = &tests[n];

	TEST(do_select_faulty_to(*tc->nfds, *tc->readfds, *tc->writefds,
				 *tc->exceptfds, *tc->timeout,
				 tc->timeout == &invalid_to));

	if (TST_RET != -1) {
		tst_res(TFAIL, "%s: select() passed unexpectedly with %ld",
		        tc->name, TST_RET);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: select()() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: select() failed as expected", tc->name);

	exit(0);
}

static void run(unsigned int n)
{
	int pid, status;

	pid = SAFE_FORK();
	if (!pid)
		verify_select(n);

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status))
		return;

	if (tst_variant == GLIBC_SELECT_VARIANT &&
	    tests[n].timeout == &invalid_to &&
	    WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "%s: select() killed by signal", tests[n].name);
		return;
	}

	tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static void setup(void)
{
	void *faulty_address;

	select_info();

	/* Regular file */
	fd_closed = SAFE_OPEN("tmpfile1", O_CREAT | O_RDWR, 0777);
	FD_ZERO(&fds_closed);
	FD_SET(fd_closed, &fds_closed);

	SAFE_PIPE(fd);
	FD_ZERO(&readfds_reg);
	FD_ZERO(&writefds_reg);
	FD_SET(fd[0], &readfds_reg);
	FD_SET(fd[1], &writefds_reg);

	SAFE_CLOSE(fd_closed);

	maxfds = fd[1] + 1;
	faulty_address = tst_get_bad_addr(NULL);
	invalid_to = faulty_address;
	faulty_fds = faulty_address;
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tests),
	.test_variants = TEST_VARIANTS,
	.setup = setup,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
