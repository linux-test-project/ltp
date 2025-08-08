/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 * Verify that:
 *     Basic test for fcntl(2) using F_GETOWN, F_SETOWN, F_GETOWN_EX,
 *     F_SETOWN_EX, F_GETSIG, F_SETSIG argument.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sched.h>

#include "test.h"
#include "config.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"

static void setup(void);
static void cleanup(void);

static void setown_pid_test(void);
static void setown_pgrp_test(void);

static void setownex_tid_test(void);
static void setownex_pid_test(void);
static void setownex_pgrp_test(void);

static struct f_owner_ex orig_own_ex;

static void signal_parent(void);
static void check_io_signal(char *des);
static void test_set_and_get_sig(int sig, char *des);

static pid_t pid;
static pid_t orig_pid;
static pid_t pgrp_pid;

static struct timespec timeout;
static sigset_t newset, oldset;

static int test_fd;
static int pipe_fds[2];

static void (*testfunc[])(void) = {
	setown_pid_test, setown_pgrp_test,
	setownex_tid_test, setownex_pid_test, setownex_pgrp_test
};

char *TCID = "fcntl31";
int TST_TOTAL = ARRAY_SIZE(testfunc);


int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int ret;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* we have these tests on pipe */
	SAFE_PIPE(cleanup, pipe_fds);
	test_fd = pipe_fds[0];
	if (fcntl(test_fd, F_SETFL, O_ASYNC) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fcntl set O_ASYNC failed");

	pid = getpid();

	/* Changing process group ID is forbidden when PID == SID i.e. we are session leader */
	if (pid != getsid(0)) {
		ret = setpgrp();
		if (ret < 0)
			tst_brkm(TBROK | TERRNO, cleanup, "setpgrp() failed");
	}
	pgrp_pid = getpgid(0);
	if (pgrp_pid < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "getpgid() failed");

	/* get original f_owner_ex info */
	TEST(fcntl(test_fd, F_GETOWN_EX, &orig_own_ex));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl get original f_owner_ex info failed");
	}

	/* get original pid info */
	TEST(fcntl(test_fd, F_GETOWN));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl get original pid info failed");
	}
	orig_pid = TEST_RETURN;

	sigemptyset(&newset);
	sigaddset(&newset, SIGUSR1);
	sigaddset(&newset, SIGIO);

	if (sigprocmask(SIG_SETMASK, &newset, &oldset) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "sigprocmask failed");

	timeout.tv_sec = 5;
	timeout.tv_nsec = 0;
}

static void setown_pid_test(void)
{
	TEST(fcntl(test_fd, F_SETOWN, pid));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl(F_SETOWN) set process id failed");
	}
	test_set_and_get_sig(SIGUSR1, "F_GETOWN, F_SETOWN for process ID");

	TEST(fcntl(test_fd, F_SETOWN, orig_pid));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl(F_SETOWN) restore orig_pid failed");
	}
}

static void setown_pgrp_test(void)
{
	TEST(fcntl(test_fd, F_SETOWN, -pgrp_pid));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl(F_SETOWN) set process group id failed");
	}
	test_set_and_get_sig(SIGUSR1,
			     "F_GETOWN, F_SETOWN for process group ID");

	TEST(fcntl(test_fd, F_SETOWN, orig_pid));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl(F_SETOWN) restore orig_pid failed");
	}
}

static void setownex_cleanup(void)
{
	TEST(fcntl(test_fd, F_SETOWN_EX, &orig_own_ex));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl F_SETOWN_EX restore orig_own_ex failed");
	}
}

static void setownex_tid_test(void)
{
	static struct f_owner_ex tst_own_ex;

	tst_own_ex.type = F_OWNER_TID;
	tst_own_ex.pid = tst_syscall(__NR_gettid);

	TEST(fcntl(test_fd, F_SETOWN_EX, &tst_own_ex));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl F_SETOWN_EX failed");
	}
	test_set_and_get_sig(SIGUSR1, "F_GETOWN_EX, F_SETOWN_EX for thread ID");

	setownex_cleanup();
}

static void setownex_pid_test(void)
{
	static struct f_owner_ex tst_own_ex;

	tst_own_ex.type = F_OWNER_PID;
	tst_own_ex.pid = pid;

	TEST(fcntl(test_fd, F_SETOWN_EX, &tst_own_ex));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl F_SETOWN_EX failed");
	}
	test_set_and_get_sig(SIGUSR1,
			     "F_GETOWN_EX, F_SETOWN_EX for process ID");

	setownex_cleanup();
}

static void setownex_pgrp_test(void)
{
	static struct f_owner_ex tst_own_ex;

	tst_own_ex.type = F_OWNER_PGRP;
	tst_own_ex.pid = pgrp_pid;

	TEST(fcntl(test_fd, F_SETOWN_EX, &tst_own_ex));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl F_SETOWN_EX failed");
	}
	test_set_and_get_sig(SIGUSR1,
			     "F_GETOWN_EX, F_SETOWN_EX for process group ID");

	setownex_cleanup();
}

static void test_set_and_get_sig(int sig, char *des)
{
	int orig_sig;

	TEST(fcntl(test_fd, F_GETSIG));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl(fd, F_GETSIG) get orig_sig failed");
	}
	orig_sig = TEST_RETURN;

	if (orig_sig == 0 || orig_sig == SIGIO)
		tst_resm(TINFO, "default io events signal is SIGIO");

	TEST(fcntl(test_fd, F_SETSIG, sig));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl(fd, F_SETSIG, SIG: %d) failed", sig);
	}

	TEST(fcntl(test_fd, F_GETSIG));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl(fd, F_GETSIG) get the set signal failed");
	}
	if (TEST_RETURN != sig) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl F_SETSIG set SIG: %d failed", sig);
	}

	check_io_signal(des);

	/* restore the default signal*/
	TEST(fcntl(test_fd, F_SETSIG, orig_sig));
	if (TEST_RETURN < 0) {
		tst_brkm(TFAIL | TTERRNO, cleanup,
			 "fcntl restore default signal failed");
	}
}

static void signal_parent(void)
{
	int ret, fd;

	fd = pipe_fds[1];
	close(pipe_fds[0]);

	ret = setpgrp();
	if (ret < 0) {
		fprintf(stderr, "child process(%d) setpgrp() failed: %s \n",
			getpid(), strerror(errno));
	}

	/* Wait for parent process to enter sigtimedwait(). */
	tst_process_state_wait2(getppid(), 'S');

	ret = write(fd, "c", 1);

	switch (ret) {
	case 0:
		fprintf(stderr, "No data written, something is wrong\n");
	break;
	case -1:
		fprintf(stderr, "Failed to write to pipe: %s\n",
			strerror(errno));
	break;
	}

	close(fd);
	return;
}

static void check_io_signal(char *des)
{
	int ret;
	char c;
	pid_t child;

	child = tst_fork();
	if (child < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fork failed");

	if (child == 0) {
		signal_parent();
		exit(0);
	} else {
		ret = sigtimedwait(&newset, NULL, &timeout);
		if (ret == -1) {
			tst_brkm(TBROK | TERRNO, NULL,
				 "sigtimedwait() failed.");
		}

		switch (ret) {
		case SIGUSR1:
			tst_resm(TPASS, "fcntl test %s success", des);
		break;
		case SIGIO:
			tst_resm(TFAIL, "received default SIGIO, fcntl test "
				 "%s failed", des);
		break;
		default:
			tst_brkm(TBROK, cleanup, "fcntl io events "
				 "signal mechanism work abnormally");
		}

		SAFE_READ(cleanup, 1, test_fd, &c, 1);
		wait(NULL);
	}
}

static void cleanup(void)
{
	if (sigprocmask(SIG_SETMASK, &oldset, NULL) < 0)
		tst_resm(TWARN | TERRNO, "sigprocmask restore oldset failed");

	if (pipe_fds[0] > 0 && close(pipe_fds[0]) == -1)
		tst_resm(TWARN | TERRNO, "close(%d) failed", pipe_fds[0]);
	if (pipe_fds[1] > 0 && close(pipe_fds[1]) == -1)
		tst_resm(TWARN | TERRNO, "close(%d) failed", pipe_fds[1]);
}
