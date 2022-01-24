// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Jiri Palecek<jpalecek@web.de>, 2009 */

#define TST_NO_DEFAULT_MAIN
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include "libsigwait.h"
#include "tst_sig_proc.h"
#include "lapi/syscalls.h"

void test_empty_set(swi_func sigwaitinfo, int signo,
		    enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs;
	siginfo_t si;
	pid_t child;

	SAFE_SIGEMPTYSET(&sigs);

	/* Run a child that will wake us up */
	child = create_sig_proc(signo, INT_MAX, 100000);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	if (TST_RET == -1) {
		if (TST_ERR == EINTR)
			tst_res(TPASS, "Wait interrupted by expected signal");
		else
			tst_res(TFAIL | TTERRNO, "Expected error number EINTR, got");
	} else {
		tst_res(TFAIL, "Expected return value -1, got: %ld", TST_RET);
	}

	SAFE_KILL(child, SIGTERM);
	SAFE_WAIT(NULL);
}

void test_timeout(swi_func sigwaitinfo, int signo, enum tst_ts_type type)
{
	sigset_t sigs;
	siginfo_t si;
	pid_t child;
	struct tst_ts ts;

	ts.type = type;
	tst_ts_set_sec(&ts, 1);
	tst_ts_set_nsec(&ts, 0);

	SAFE_SIGEMPTYSET(&sigs);

	/* Run a child that will wake us up */
	child = create_sig_proc(signo, INT_MAX, 100000);

	TEST(sigwaitinfo(&sigs, &si, tst_ts_get(&ts)));
	if (TST_RET == -1) {
		if (TST_ERR == EAGAIN)
			tst_res(TPASS, "Wait interrupted by timeout");
		else
			tst_res(TFAIL | TTERRNO, "Expected error number EAGAIN, got");
	} else {
		tst_res(TFAIL, "Expected return value -1, got: %ld", TST_RET);
	}

	SAFE_KILL(child, SIGTERM);
	SAFE_WAIT(NULL);
}

/* Note: sigwait-ing for a signal that is not blocked is unspecified
 * by POSIX; but works for non-ignored signals under Linux
 */
void test_unmasked_matching(swi_func sigwaitinfo, int signo,
			    enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs;
	siginfo_t si;
	pid_t child;

	SAFE_SIGEMPTYSET(&sigs);
	SAFE_SIGADDSET(&sigs, signo);

	/* Run a child that will wake us up */
	child = create_sig_proc(signo, INT_MAX, 100000);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	if (TST_RET == signo) {
		if (si.si_pid == child && si.si_code == SI_USER &&
		    si.si_signo == signo)
			tst_res(TPASS, "struct siginfo is correct");
		else
			tst_res(TFAIL, "struct siginfo mismatch");
	} else {
		tst_res(TFAIL | TTERRNO, "sigwaitinfo() failed");
	}

	SAFE_KILL(child, SIGTERM);
	SAFE_WAIT(NULL);
}

void test_unmasked_matching_noinfo(swi_func sigwaitinfo, int signo,
				   enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs;
	pid_t child;

	SAFE_SIGEMPTYSET(&sigs);
	SAFE_SIGADDSET(&sigs, signo);

	/* Run a child that will wake us up */
	child = create_sig_proc(signo, INT_MAX, 100000);

	TEST(sigwaitinfo(&sigs, NULL, NULL));
	if (TST_RET == signo)
		tst_res(TPASS, "Wait interrupted by expected signal");
	else
		tst_res(TFAIL | TTERRNO, "sigwaitinfo() failed");

	SAFE_KILL(child, SIGTERM);
	SAFE_WAIT(NULL);
}

void test_masked_matching(swi_func sigwaitinfo, int signo,
			  enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs, oldmask;
	siginfo_t si;
	pid_t child;

	SAFE_SIGEMPTYSET(&sigs);
	SAFE_SIGADDSET(&sigs, signo);

	/* let's not get interrupted by our dying child */
	SAFE_SIGADDSET(&sigs, SIGCHLD);

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	SAFE_SIGDELSET(&sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child = create_sig_proc(signo, 1, 0);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	if (TST_RET == signo) {
		if (si.si_pid == child && si.si_code == SI_USER &&
		    si.si_signo == signo)
			tst_res(TPASS, "struct siginfo is correct");
		else
			tst_res(TFAIL, "struct siginfo mismatch");
	} else {
		tst_res(TFAIL | TTERRNO, "sigwaitinfo() failed");
	}

	TEST(sigprocmask(SIG_SETMASK, &oldmask, &sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "restoring original signal mask failed");

	if (sigismember(&sigs, signo))
		tst_res(TPASS, "sigwaitinfo restored the original mask");
	else
		tst_res(TFAIL,
			 "sigwaitinfo failed to restore the original mask");

	SAFE_KILL(child, SIGTERM);
	SAFE_WAIT(NULL);
}

void test_masked_matching_rt(swi_func sigwaitinfo, int signo,
			     enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs, oldmask;
	siginfo_t si;
	pid_t child[2];
	int status;

	signo = SIGRTMIN + 1;

	SAFE_SIGEMPTYSET(&sigs);
	SAFE_SIGADDSET(&sigs, signo);
	SAFE_SIGADDSET(&sigs, signo + 1);

	/* let's not get interrupted by our dying child */
	SAFE_SIGADDSET(&sigs, SIGCHLD);

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	SAFE_SIGDELSET(&sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child[0] = create_sig_proc(signo, 1, 0);
	child[1] = create_sig_proc(signo + 1, 1, 0);

	/* Ensure that the signals have been sent */
	SAFE_WAITPID(child[0], &status, 0);
	SAFE_WAITPID(child[1], &status, 0);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	if (TST_RET == signo) {
		if (si.si_pid == child[0] && si.si_code == SI_USER &&
		    si.si_signo == signo)
			tst_res(TPASS, "struct siginfo is correct");
		else
			tst_res(TFAIL, "struct siginfo mismatch");
	} else {
		tst_res(TFAIL | TTERRNO, "sigwaitinfo() failed");
	}

	/* eat the other signal */
	TEST(sigwaitinfo(&sigs, &si, NULL));
	if (TST_RET == signo + 1) {
		if (si.si_pid == child[1] && si.si_code == SI_USER &&
		    si.si_signo == signo + 1)
			tst_res(TPASS, "struct siginfo is correct");
		else
			tst_res(TFAIL, "struct siginfo mismatch");
	} else {
		tst_res(TFAIL | TTERRNO, "sigwaitinfo() failed");
	}

	TEST(sigprocmask(SIG_SETMASK, &oldmask, &sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "restoring original signal mask failed");

	if (sigismember(&sigs, signo))
		tst_res(TPASS, "sigwaitinfo restored the original mask");
	else
		tst_res(TFAIL,
			 "sigwaitinfo failed to restore the original mask");
}

void test_masked_matching_noinfo(swi_func sigwaitinfo, int signo,
				 enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs, oldmask;
	pid_t child;

	SAFE_SIGEMPTYSET(&sigs);
	SAFE_SIGADDSET(&sigs, signo);

	/* let's not get interrupted by our dying child */
	SAFE_SIGADDSET(&sigs, SIGCHLD);

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	SAFE_SIGDELSET(&sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child = create_sig_proc(signo, 1, 0);

	TEST(sigwaitinfo(&sigs, NULL, NULL));
	if (TST_RET == signo)
		tst_res(TPASS, "Wait interrupted by expected signal");
	else
		tst_res(TFAIL | TTERRNO, "sigwaitinfo() failed");

	TEST(sigprocmask(SIG_SETMASK, &oldmask, &sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "restoring original signal mask failed");

	if (sigismember(&sigs, signo))
		tst_res(TPASS, "sigwaitinfo restored the original mask");
	else
		tst_res(TFAIL,
			 "sigwaitinfo failed to restore the original mask");

	SAFE_KILL(child, SIGTERM);
	SAFE_WAIT(NULL);
}

void test_bad_address(swi_func sigwaitinfo, int signo,
		      enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs, oldmask;
	pid_t child;

	SAFE_SIGEMPTYSET(&sigs);
	SAFE_SIGADDSET(&sigs, signo);

	/* let's not get interrupted by our dying child */
	SAFE_SIGADDSET(&sigs, SIGCHLD);

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	SAFE_SIGDELSET(&sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child = create_sig_proc(signo, 1, 0);

	TEST(sigwaitinfo(&sigs, (void *)1, NULL));
	if (TST_RET == -1) {
		if (TST_ERR == EFAULT)
			tst_res(TPASS, "Fault occurred while accessing the buffers");
		else
			tst_res(TFAIL | TTERRNO, "Expected error number EFAULT, got");
	} else {
		tst_res(TFAIL, "Expected return value -1, got: %ld", TST_RET);
	}

	TEST(sigprocmask(SIG_SETMASK, &oldmask, NULL));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "restoring original signal mask failed");

	SAFE_KILL(child, SIGTERM);
	SAFE_WAIT(NULL);
}

void test_bad_address2(swi_func sigwaitinfo, int signo LTP_ATTRIBUTE_UNUSED,
		       enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	pid_t pid;
	int status;

	pid = SAFE_FORK();
	if (pid == 0) {
		signal(SIGSEGV, SIG_DFL);

		/*
		 * depending on glibc implementation we should
		 * either crash or get EFAULT
		 */
		TEST(sigwaitinfo((void *)1, NULL, NULL));

		if (TST_RET == -1 && TST_ERR == EFAULT)
			_exit(0);

		tst_res(TINFO | TTERRNO, "swi_func returned: %ld", TST_RET);
		_exit(1);
	}

	SAFE_WAITPID(pid, &status, 0);

	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
		tst_res(TPASS, "Child exited with expected code");
		return;
	}

	if (WIFEXITED(status)) {
		tst_res(TFAIL, "Unrecognised child exit code: %d",
			WEXITSTATUS(status));
	}
	if (WIFSIGNALED(status)) {
		tst_res(TFAIL, "Unrecognised child termsig: %d",
			WTERMSIG(status));
	}
}

void test_bad_address3(swi_func sigwaitinfo, int signo LTP_ATTRIBUTE_UNUSED,
		       enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs;

	SAFE_SIGEMPTYSET(&sigs);
	TEST(sigwaitinfo(&sigs, NULL, (void *)1));
	if (TST_RET == -1) {
		if (TST_ERR == EFAULT)
			tst_res(TPASS, "Fault occurred while accessing the buffers");
		else
			tst_res(TFAIL | TTERRNO, "Expected error number EFAULT, got");
	} else {
		tst_res(TFAIL, "Expected return value -1, got: %ld", TST_RET);
	}
}

static void empty_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

void sigwait_setup(void)
{
	signal(SIGUSR1, empty_handler);
	signal(SIGALRM, empty_handler);
	signal(SIGUSR2, SIG_IGN);
}
