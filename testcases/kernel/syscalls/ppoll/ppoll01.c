/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "lapi/syscalls.h"
#include "ltp_signal.h"
#include "tst_sig_proc.h"
#include "tst_test.h"

/* Older versions of glibc don't publish this constant's value. */
#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
#endif

#define TYPE_NAME(x) .ttype = x, .desc = #x

struct test_case {
	int ttype;		   /* test type (enum) */
	const char *desc;	   /* test description (name) */
	int ret;		   /* expected ret code */
	int err;		   /* expected errno code */
	short expect_revents;	   /* expected revents value */
	unsigned int nfds;	   /* nfds ppoll parameter */
	sigset_t *sigmask;	   /* sigmask ppoll parameter */
	sigset_t *sigmask_cur;	   /* sigmask set for current process */
	struct timespec *ts;	   /* ts ppoll parameter */
	struct pollfd *fds;	   /* fds ppoll parameter */
	int sigint_count;	   /* if > 0, spawn process to send SIGINT */
				   /* 'count' times to current process */
	unsigned int sigint_delay; /* delay between SIGINT signals */
};

enum test_type {
	NORMAL,
	MASK_SIGNAL,
	TIMEOUT,
	FD_ALREADY_CLOSED,
	SEND_SIGINT,
	SEND_SIGINT_RACE_TEST,
	INVALID_NFDS,
	INVALID_FDS,
};

static int fd1 = -1;
static sigset_t sigmask_empty, sigmask_sigint;
static struct pollfd fds_good[1], fds_already_closed[1];

static struct timespec ts_short = {
	.tv_sec = 0,
	.tv_nsec = 200000000,
};
static struct timespec ts_long = {
	.tv_sec = 2,
	.tv_nsec = 0,
};

/* Test cases
 *
 *   test status of errors on man page
 *
 *   EBADF              can't check because EBADF never happen even though
 *                      fd was invalid. In this case, information of invalid
 *                      fd is set in revents
 *   EFAULT             v ('fds' array in the invalid address space)
 *   EINTR              v (a non blocked signal was caught)
 *   EINVAL             v ('nfds' is over the 'RLIMIT_NOFILE' value)
 *   ENOMEM             can't check because it's difficult to create no-memory
 */

static struct test_case tcase[] = {
	{
		TYPE_NAME(NORMAL),
		.expect_revents = POLLIN | POLLOUT,
		.ret = 1,
		.err = 0,
		.nfds = 1,
		.ts = &ts_long,
		.fds = fds_good,
	},
	{
		TYPE_NAME(MASK_SIGNAL),
		.ret = 0,
		.err = 0,
		.nfds = 0,
		.sigmask = &sigmask_sigint,
		.ts = &ts_short,
		.fds = fds_good,
		.sigint_count = 4,
		.sigint_delay = 100000,
	},
	{
		TYPE_NAME(TIMEOUT),
		.ret = 0,
		.err = 0,
		.nfds = 0,
		.ts = &ts_short,
		.fds = fds_good,
	},
	{
		TYPE_NAME(FD_ALREADY_CLOSED),
		.expect_revents = POLLNVAL,
		.ret = 1,
		.err = 0,
		.nfds = 1,
		.ts = &ts_long,
		.fds = fds_already_closed,
	},
	{
		TYPE_NAME(SEND_SIGINT),
		.ret = -1,
		.err = EINTR,
		.nfds = 0,
		.ts = &ts_long,
		.fds = fds_good,
		.sigint_count = 40,
		.sigint_delay = 100000,
	},
	{
		TYPE_NAME(SEND_SIGINT_RACE_TEST),
		.ret = -1,
		.err = EINTR,
		.nfds = 0,
		.sigmask = &sigmask_empty,
		.sigmask_cur = &sigmask_sigint,
		.ts = &ts_long,
		.fds = fds_good,
		.sigint_count = 1,
		.sigint_delay = 0,
	},
	{
		TYPE_NAME(INVALID_NFDS),
		.ret = -1,
		.err = EINVAL,
		.nfds = -1,
		.ts = &ts_long,
		.fds = fds_good,
	},
	{
		TYPE_NAME(INVALID_FDS),
		.ret = -1,
		.err = EFAULT,
		.nfds = 1,
		.ts = &ts_long,
		.fds = (struct pollfd *) -1,
	},
};

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void setup(void)
{
	int fd2;

	SAFE_SIGNAL(SIGINT, sighandler);

	if (sigemptyset(&sigmask_empty) == -1)
		tst_brk(TBROK | TERRNO, "sigemptyset");
	if (sigemptyset(&sigmask_sigint) == -1)
		tst_brk(TBROK | TERRNO, "sigemptyset");
	if (sigaddset(&sigmask_sigint, SIGINT) == -1)
		tst_brk(TBROK | TERRNO, "sigaddset");

	fd1 = SAFE_OPEN("testfile1", O_CREAT | O_EXCL | O_RDWR,
		S_IRUSR | S_IWUSR);
	fds_good[0].fd = fd1;
	fds_good[0].events = POLLIN | POLLPRI | POLLOUT | POLLRDHUP;
	fds_good[0].revents = 0;

	fd2 = SAFE_OPEN("testfile2", O_CREAT | O_EXCL | O_RDWR,
		S_IRUSR | S_IWUSR);
	fds_already_closed[0].fd = fd2;
	fds_already_closed[0].events = POLLIN | POLLPRI | POLLOUT | POLLRDHUP;
	fds_already_closed[0].revents = 0;
	SAFE_CLOSE(fd2);
}

static void cleanup(void)
{
	if (fd1 != -1)
		SAFE_CLOSE(fd1);
}

static void do_test(unsigned int i)
{
	pid_t pid = 0;
	int sys_ret, sys_errno = 0, dummy;
	struct test_case *tc = &tcase[i];
	struct timespec ts, *tsp = NULL;

	if (tc->ts) {
		memcpy(&ts, tc->ts, sizeof(ts));
		tsp = &ts;
	}

	tst_res(TINFO, "case %s", tc->desc);

	/* setup */
	if (tc->sigmask_cur) {
	       if (sigprocmask(SIG_SETMASK, tc->sigmask_cur, NULL) == -1)
			tst_brk(TBROK, "sigprocmask");
	}
	if (tc->sigint_count > 0) {
		pid = create_sig_proc(SIGINT, tc->sigint_count,
			tc->sigint_delay);
	}

	/* test */
	errno = 0;
	sys_ret = tst_syscall(__NR_ppoll, tc->fds, tc->nfds, tsp,
		tc->sigmask, SIGSETSIZE);
	sys_errno = errno;

	/* cleanup */
	if (tc->sigmask_cur) {
		if (sigprocmask(SIG_SETMASK, &sigmask_empty, NULL) == -1)
			tst_brk(TBROK, "sigprocmask");
	}
	if (pid > 0) {
		kill(pid, SIGTERM);
		SAFE_WAIT(&dummy);
	}

	/* result check */
	if (tc->expect_revents) {
		if (tc->fds[0].revents == tc->expect_revents)
			tst_res(TPASS, "revents=0x%04x", tc->expect_revents);
		else
			tst_res(TFAIL, "revents=0x%04x, expected=0x%04x",
				tc->fds[0].revents, tc->expect_revents);
	}
	if (tc->ret >= 0 && tc->ret == sys_ret) {
		tst_res(TPASS, "ret: %d", sys_ret);
	} else if (tc->ret == -1 && sys_ret == -1 && sys_errno == tc->err) {
		tst_res(TPASS, "ret: %d, errno: %s (%d)", sys_ret,
			tst_strerrno(sys_errno), sys_errno);
	} else {
		tst_res(TFAIL, "ret: %d, exp: %d, ret_errno: %s (%d),"
			" exp_errno: %s (%d)", sys_ret, tc->ret,
			tst_strerrno(sys_errno), sys_errno,
			tst_strerrno(tc->err), tc->err);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_tmpdir = 1,
};
