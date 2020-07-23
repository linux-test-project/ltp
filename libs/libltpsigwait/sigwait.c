// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Jiri Palecek<jpalecek@web.de>, 2009 */

#define TST_NO_DEFAULT_MAIN
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include "lapi/syscalls.h"
#include "libsigwait.h"
#include "tst_sig_proc.h"

/* Report success iff TST_RET and TST_ERR are equal to
	 exp_return and exp_errno, resp., and cond is true. If cond is not
	 true, report condition_errmsg
*/
static void report_success_cond(const char *func, int line,
				long exp_return, int exp_errno, int condition,
				char *condition_errmsg)
{
	if (exp_return == TST_RET
	    && (exp_return != -1 || exp_errno == TST_ERR))
		if (condition)
			tst_res(TPASS, "%s (%d): Test passed", func, line);
		else
			tst_res(TFAIL, "%s (%d): %s", func, line,
				 condition_errmsg);
	else if (TST_RET != -1)
		tst_res(TFAIL,
			 "%s (%d): Unexpected return value; expected %ld, got %ld",
			 func, line, exp_return, TST_RET);
	else
		tst_res(TFAIL | TTERRNO, "%s (%d): Unexpected failure",
			 func, line);
}

#define REPORT_SUCCESS_COND(exp_return, exp_errno, condition, condition_errmsg)	\
	report_success_cond(__FUNCTION__, __LINE__, exp_return, exp_errno, condition, condition_errmsg);

/* Report success iff TST_RET and TST_ERR are equal to
	 exp_return and exp_errno, resp.
*/
#define REPORT_SUCCESS(exp_return, exp_errno)					\
	REPORT_SUCCESS_COND(exp_return, exp_errno, 1, "");

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
	REPORT_SUCCESS(-1, EINTR);

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
	REPORT_SUCCESS(-1, EAGAIN);

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
	REPORT_SUCCESS_COND(signo, 0, si.si_pid == child
			    && si.si_code == SI_USER
			    && si.si_signo == signo, "Struct siginfo mismatch");

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
	REPORT_SUCCESS(signo, 0);

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
	REPORT_SUCCESS_COND(signo, 0, si.si_pid == child
			    && si.si_code == SI_USER
			    && si.si_signo == signo, "Struct siginfo mismatch");

	TEST(sigprocmask(SIG_SETMASK, &oldmask, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "restoring original signal mask failed");

	if (sigismember(&oldmask, signo))
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
	REPORT_SUCCESS_COND(signo, 0, si.si_pid == child[0]
			    && si.si_code == SI_USER
			    && si.si_signo == signo, "Struct siginfo mismatch");

	/* eat the other signal */
	TEST(sigwaitinfo(&sigs, &si, NULL));
	REPORT_SUCCESS_COND(signo + 1, 0, si.si_pid == child[1]
			    && si.si_code == SI_USER
			    && si.si_signo == signo + 1,
			    "Struct siginfo mismatch");

	TEST(sigprocmask(SIG_SETMASK, &oldmask, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "restoring original signal mask failed");

	if (sigismember(&oldmask, signo))
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
	REPORT_SUCCESS(signo, 0);

	TEST(sigprocmask(SIG_SETMASK, &oldmask, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "restoring original signal mask failed");

	if (sigismember(&oldmask, signo))
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
	REPORT_SUCCESS(-1, EFAULT);

	TEST(sigprocmask(SIG_SETMASK, &oldmask, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

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

		tst_res(TINFO | TTERRNO, "swi_func returned: %ld",
			TST_RET);
		_exit(1);
	}

	SAFE_WAITPID(pid, &status, 0);

	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
		tst_res(TPASS, "Test passed");
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
	REPORT_SUCCESS(-1, EFAULT);
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
