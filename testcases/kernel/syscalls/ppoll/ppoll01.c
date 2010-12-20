/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd                */
/*          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,	      */
/*		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>, 	      */
/*		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>	      */
/*                                                                  	      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        ppoll01.c                                                     */
/*                                                                            */
/* Description: This tests the ppoll01() syscall                              */
/*									      */
/* 									      */
/*									      */
/*									      */
/*									      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* ppoll01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                               */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   ppoll01                                                       */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <poll.h>

#include "../utils/include_j_h.h"
#include "../utils/common_j_h.c"

/* Harness Specific Include Files. */
#include "ltp_signal.h"
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Older versions of glibc don't publish this constant's value. */
#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
#endif

char *TCID = "ppoll01"; /* Test program identifier.*/
int testno;
int TST_TOTAL = 1; /* total number of tests in this file.   */

void
sighandler(int sig)
{
	if (sig == SIGINT)
		return;
	else
		tst_brkm(TBROK, NULL, "received unexpected signal %d", sig);
}

void cleanup() {

	TEST_CLEANUP;
	tst_rmdir();
}

void setup() {
	tst_sig(FORK, sighandler, cleanup);

	TEST_PAUSE;
	tst_tmpdir();
}

/*
 * Macros
 */
#define SYSCALL_NAME    "ppoll"

/*
 * Global variables
 */

enum test_type {
	NORMAL,
	MASK_SIGNAL,
	TIMEOUT,
	FD_ALREADY_CLOSED,
	SEND_SIGINT,
	SEND_SIGINT_RACE_TEST,
	INVALID_NFDS,
	INVALID_FDS,
	MINUS_NSEC,
	TOO_LARGE_NSEC,
};

/*
 * Data Structure
 */
struct test_case {
	short expect_revents;
	int ttype;
	int ret;
	int err;
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
		{ // case00
			.ttype = NORMAL,
			.expect_revents = POLLIN | POLLOUT,
			.ret = 0,
			.err = 0,
		},
		{ // case01
			.ttype = MASK_SIGNAL,
			.expect_revents = 0, // don't care
			.ret = 0,
			.err = 0,
		},
		{ // case02
			.ttype = TIMEOUT,
			.expect_revents = 0, // don't care
			.ret = 0,
			.err = 0,
		},
		{ // case03
			.ttype = FD_ALREADY_CLOSED,
			.expect_revents = POLLNVAL,
			.ret = 0, .err = 0,
		},
		{ // case04
			.ttype = SEND_SIGINT,
			.ret = -1,
			.err = EINTR,
		},
		{ // case05
			.ttype = SEND_SIGINT_RACE_TEST,
			.ret = -1,
			.err = EINTR,
		},
		{ // case06
			.ttype = INVALID_NFDS,
			.ret = -1,
			.err = EINVAL,
		},
		{ // case07
			.ttype = INVALID_FDS,
			.ret = -1,
			.err = EFAULT, },
#if 0
		{ // case08
			.ttype = MINUS_NSEC,
			.ret = -1,
			.err = EINVAL, // RHEL4U1 + 2.6.18 returns SUCCESS
		},
		{ // case09
			.ttype = TOO_LARGE_NSEC,
			.ret = -1,
			.err = EINVAL, // RHEL4U1 + 2.6.18 returns SUCCESS
		},
#endif
		};

#define NUM_TEST_FDS    1

/*
 * do_test()
 *
 *   Input  : TestCase Data
 *   Return : RESULT_OK(0), RESULT_NG(1)
 *
 */

static int do_test(struct test_case *tc) {
	int sys_ret, sys_errno;
	int result = RESULT_OK;
	int fd = -1, cmp_ok = 1;
	char fpath[PATH_MAX];
	struct pollfd *p_fds, fds[NUM_TEST_FDS];
	unsigned int nfds = NUM_TEST_FDS;
	struct timespec *p_ts, ts;
	sigset_t *p_sigmask, sigmask;
	pid_t pid = 0;

	TEST(fd = setup_file(".", "test.file", fpath));
	if (fd < 0)
		return 1;
	fds[0].fd = fd;
	fds[0].events = POLLIN | POLLPRI | POLLOUT | POLLRDHUP;
	fds[0].revents = 0;
	p_fds = fds;
	p_ts = NULL;
	p_sigmask = NULL;

	switch (tc->ttype) {
	case TIMEOUT:
		nfds = 0;
		ts.tv_sec = 0;
		ts.tv_nsec = 50000000; // 50msec
		p_ts = &ts;
		break;
	case FD_ALREADY_CLOSED:
		TEST(close(fd));
		fd = -1;
		TEST(cleanup_file(fpath));
		break;
	case MASK_SIGNAL:
		TEST(sigemptyset(&sigmask));
		TEST(sigaddset(&sigmask, SIGINT));
		p_sigmask = &sigmask;
		nfds = 0;
		ts.tv_sec = 0;
		ts.tv_nsec = 300000000; // 300msec => need to be enough for
		//   waiting the signal
		p_ts = &ts;
		// fallthrough
	case SEND_SIGINT:
		nfds = 0;
		pid = create_sig_proc(100000, SIGINT, UINT_MAX); // 100msec
		if (pid < 0)
			return 1;
		break;
	case SEND_SIGINT_RACE_TEST:
		/* block the INT signal */
		sigemptyset(&sigmask);
		sigaddset(&sigmask, SIGINT);
		sigprocmask(SIG_SETMASK, &sigmask, NULL);

		/* and let it be unblocked when the syscall runs */
		sigemptyset(&sigmask);
		p_sigmask = &sigmask;
		nfds = 0;
		ts.tv_sec = 0;
		ts.tv_nsec = 300000000; // 300msec => need to be enough for
		//   waiting the signal
		p_ts = &ts;
		nfds = 0;
		pid = create_sig_proc(0, SIGINT, 1);
		if (pid < 0) {
			result = 1;
			goto cleanup;
		}
		break;
	case INVALID_NFDS:
		//nfds = RLIMIT_NOFILE + 1; //==> RHEL4U1 + 2.6.18 returns SUCCESS
		nfds = -1;
		break;
	case INVALID_FDS:
		p_fds = (void*) 0xc0000000;
		break;
	case MINUS_NSEC:
		ts.tv_sec = 0;
		ts.tv_nsec = -1;
		p_ts = &ts;
		break;
	case TOO_LARGE_NSEC:
		ts.tv_sec = 0;
		ts.tv_nsec = 1000000000;
		p_ts = &ts;
		break;
	}

	/*
	 * Execute system call
	 */
	errno = 0;
	sys_ret = syscall(__NR_ppoll, p_fds, nfds, p_ts, p_sigmask, SIGSETSIZE);
	sys_errno = errno;
	if (sys_ret <= 0 || tc->ret < 0)
		goto TEST_END;

	cmp_ok = fds[0].revents == tc->expect_revents;
	tst_resm(TINFO, "EXPECT: revents=0x%04x", tc->expect_revents);
	tst_resm(TINFO, "RESULT: revents=0x%04x", fds[0].revents);

TEST_END:
	/*
	 * Check results
	 */
	if (tc->ttype == SEND_SIGINT_RACE_TEST) {
		int sig;
		sigprocmask(SIG_SETMASK, NULL, &sigmask);
		for (sig = 1; sig < SIGRTMAX; sig++) {
			TEST(sigismember(&sigmask, sig));
			if (TEST_RETURN < 0 && TEST_ERRNO == EINVAL && sig != SIGINT)
				continue; /* let's ignore errors if they are for other signal than SIGINT that we set */
			if ((sig == SIGINT) != (TEST_RETURN != 0)) {
				tst_resm(TFAIL, "Bad value of signal mask, signal %d is %s",
						sig, TEST_RETURN ? "on" : "off");
				cmp_ok |= 1;
			}
		}
	}
	result |= (sys_errno != tc->err) || !cmp_ok;
	PRINT_RESULT_CMP(sys_ret >= 0, tc->ret, tc->err, sys_ret, sys_errno, cmp_ok);
cleanup:
	if (fd >= 0) {
		close(fd);
		cleanup_file(fpath);
	}

	sigemptyset(&sigmask);
	sigprocmask(SIG_SETMASK, &sigmask, NULL);
	if (pid > 0) {
		int st;
		kill(pid, SIGTERM);
		wait(&st);
	}
	return result;
}

/*
 * main()
 */

int main(int ac, char **av) {
	int i;
	int ret;

	setup();

	ret = 0;

	for (i = 0; ret == 0 &&
	    i < (sizeof(tcase) / sizeof(tcase[0])); i++) {
		tst_resm(TINFO, "(case%02d) START", i);
		ret = do_test(&tcase[i]);
		tst_resm(TINFO, "(case%02d) END => %s", i, (ret == 0) ? "OK"
				: "NG");
	}
	cleanup();
	tst_exit();
}
