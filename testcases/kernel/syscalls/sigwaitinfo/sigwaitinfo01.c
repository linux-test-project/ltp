/*
 * Copyright (c) Jiri Palecek<jpalecek@web.de>, 2009
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "test.h"
#include <errno.h>
#include <signal.h>
#include "../utils/include_j_h.h"
#include "../utils/common_j_h.c"
#include <limits.h>
#include "lapi/syscalls.h"

#define SUCCEED_OR_DIE(syscall, message, ...)				 \
	(errno = 0,							 \
		({int ret=syscall(__VA_ARGS__);				 \
			if (ret==-1)					 \
				tst_brkm(TBROK|TERRNO, cleanup, message);\
			ret;}))

/* Report success iff TEST_RETURN and TEST_ERRNO are equal to
	 exp_return and exp_errno, resp., and cond is true. If cond is not
	 true, report condition_errmsg
*/
static void report_success_cond(const char *func, const char *file, int line,
				long exp_return, int exp_errno, int condition,
				char *condition_errmsg)
{
	if (exp_return == TEST_RETURN
	    && (exp_return != -1 || exp_errno == TEST_ERRNO))
		if (condition)
			tst_resm(TPASS, "Test passed");
		else
			tst_resm(TFAIL, "%s (%s: %d): %s", func, file, line,
				 condition_errmsg);
	else if (TEST_RETURN != -1)
		tst_resm(TFAIL,
			 "%s (%s: %d): Unexpected return value; expected %ld, got %ld",
			 func, file, line, exp_return, TEST_RETURN);
	else
		tst_resm(TFAIL | TTERRNO, "%s (%s: %d): Unexpected failure",
			 func, file, line);
}

#define REPORT_SUCCESS_COND(exp_return, exp_errno, condition, condition_errmsg)	\
	report_success_cond(__FUNCTION__, __FILE__, __LINE__, exp_return, exp_errno, condition, condition_errmsg);

/* Report success iff TEST_RETURN and TEST_ERRNO are equal to
	 exp_return and exp_errno, resp.
*/
#define REPORT_SUCCESS(exp_return, exp_errno)					\
	REPORT_SUCCESS_COND(exp_return, exp_errno, 1, "");

static void cleanup(void);

static void empty_handler(int sig)
{
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	signal(SIGUSR1, empty_handler);
	signal(SIGALRM, empty_handler);
	signal(SIGUSR2, SIG_IGN);

	TEST_PAUSE;
}

static void cleanup(void)
{
}

typedef int (*swi_func) (const sigset_t * set, siginfo_t * info,
			 struct timespec * timeout);
typedef void (*test_func) (swi_func, int);

#ifdef TEST_SIGWAIT
static int my_sigwait(const sigset_t * set, siginfo_t * info,
		      struct timespec *timeout)
{
	int ret;
	int err = sigwait(set, &ret);

	if (err == 0)
		return ret;
	errno = err;
	return -1;
}
#endif

#ifdef TEST_SIGWAITINFO
static int my_sigwaitinfo(const sigset_t * set, siginfo_t * info,
			  struct timespec *timeout)
{

	return sigwaitinfo(set, info);
}
#endif

#ifdef TEST_SIGTIMEDWAIT
static int my_sigtimedwait(const sigset_t * set, siginfo_t * info,
			   struct timespec *timeout)
{

	return sigtimedwait(set, info, timeout);
}
#endif

#ifdef TEST_RT_SIGTIMEDWAIT
static int my_rt_sigtimedwait(const sigset_t * set, siginfo_t * info,
			      struct timespec *timeout)
{
	/* _NSIG is always the right number of bits of signal map for all arches */
	return ltp_syscall(__NR_rt_sigtimedwait, set, info, timeout, _NSIG/8);
}
#endif

void test_empty_set(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs;
	siginfo_t si;
	pid_t child;

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);
	/* Run a child that will wake us up */
	child = create_sig_proc(100000, signo, UINT_MAX);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	REPORT_SUCCESS(-1, EINTR);

	kill(child, SIGTERM);
}

void test_timeout(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs;
	siginfo_t si;
	pid_t child;
	struct timespec ts = {.tv_sec = 1 };

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);

	/* Run a child that will wake us up */
	child = create_sig_proc(100000, signo, UINT_MAX);

	TEST(sigwaitinfo(&sigs, &si, &ts));
	REPORT_SUCCESS(-1, EAGAIN);

	kill(child, SIGTERM);
}

/* Note: sigwait-ing for a signal that is not blocked is unspecified
 * by POSIX; but works for non-ignored signals under Linux
 */
void test_unmasked_matching(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs;
	siginfo_t si;
	pid_t child;

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, signo);

	/* Run a child that will wake us up */
	child = create_sig_proc(100000, signo, UINT_MAX);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	REPORT_SUCCESS_COND(signo, 0, si.si_pid == child
			    && si.si_code == SI_USER
			    && si.si_signo == signo, "Struct siginfo mismatch");

	kill(child, SIGTERM);
}

void test_unmasked_matching_noinfo(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs;
	pid_t child;

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, signo);
	/* Run a child that will wake us up */
	child = create_sig_proc(100000, signo, UINT_MAX);

	TEST(sigwaitinfo(&sigs, NULL, NULL));
	REPORT_SUCCESS(signo, 0);

	kill(child, SIGTERM);
}

void test_masked_matching(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs, oldmask;
	siginfo_t si;
	pid_t child;

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, signo);
	/* let's not get interrupted by our dying child */
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, SIGCHLD);

	SUCCEED_OR_DIE(sigprocmask, "sigprocmask failed", SIG_SETMASK, &sigs,
		       &oldmask);

	/* don't wait on a SIGCHLD */
	SUCCEED_OR_DIE(sigdelset, "sigaddset failed", &sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child = create_sig_proc(0, signo, 1);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	REPORT_SUCCESS_COND(signo, 0, si.si_pid == child
			    && si.si_code == SI_USER
			    && si.si_signo == signo, "Struct siginfo mismatch");

	SUCCEED_OR_DIE(sigprocmask, "restoring original signal mask failed",
		       SIG_SETMASK, &oldmask, &oldmask);

	tst_count--;

	if (sigismember(&oldmask, signo))
		tst_resm(TPASS, "sigwaitinfo restored the original mask");
	else
		tst_resm(TFAIL,
			 "sigwaitinfo failed to restore the original mask");
}

void test_masked_matching_rt(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs, oldmask;
	siginfo_t si;
	pid_t child[2];
	int status;

	signo = SIGRTMIN + 1;

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, signo);
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, signo + 1);
	/* let's not get interrupted by our dying child */
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, SIGCHLD);

	SUCCEED_OR_DIE(sigprocmask, "sigprocmask failed", SIG_SETMASK, &sigs,
		       &oldmask);

	/* don't wait on a SIGCHLD */
	SUCCEED_OR_DIE(sigdelset, "sigdelset failed", &sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child[0] = create_sig_proc(0, signo, 1);
	child[1] = create_sig_proc(0, signo + 1, 1);

	/* Ensure that the signals have been sent */
	waitpid(child[0], &status, 0);
	waitpid(child[1], &status, 0);

	TEST(sigwaitinfo(&sigs, &si, NULL));
	REPORT_SUCCESS_COND(signo, 0, si.si_pid == child[0]
			    && si.si_code == SI_USER
			    && si.si_signo == signo, "Struct siginfo mismatch");

	/* eat the other signal */
	tst_count--;
	TEST(sigwaitinfo(&sigs, &si, NULL));
	REPORT_SUCCESS_COND(signo + 1, 0, si.si_pid == child[1]
			    && si.si_code == SI_USER
			    && si.si_signo == signo + 1,
			    "Struct siginfo mismatch");

	SUCCEED_OR_DIE(sigprocmask, "restoring original signal mask failed",
		       SIG_SETMASK, &oldmask, &oldmask);

	tst_count--;

	if (sigismember(&oldmask, signo))
		tst_resm(TPASS, "sigwaitinfo restored the original mask");
	else
		tst_resm(TFAIL,
			 "sigwaitinfo failed to restore the original mask");
}

void test_masked_matching_noinfo(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs, oldmask;
	pid_t child;

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, signo);
	/* let's not get interrupted by our dying child */
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, SIGCHLD);

	SUCCEED_OR_DIE(sigprocmask, "sigprocmask failed", SIG_SETMASK, &sigs,
		       &oldmask);

	/* don't wait on a SIGCHLD */
	SUCCEED_OR_DIE(sigdelset, "sigaddset failed", &sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child = create_sig_proc(0, signo, 1);

	TEST(sigwaitinfo(&sigs, NULL, NULL));
	REPORT_SUCCESS(signo, 0);

	SUCCEED_OR_DIE(sigprocmask, "restoring original signal mask failed",
		       SIG_SETMASK, &oldmask, &oldmask);

	tst_count--;

	if (sigismember(&oldmask, signo))
		tst_resm(TPASS, "sigwaitinfo restored the original mask");
	else
		tst_resm(TFAIL,
			 "sigwaitinfo failed to restore the original mask");

}

void test_bad_address(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs, oldmask;
	pid_t child;

	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, signo);
	/* let's not get interrupted by our dying child */
	SUCCEED_OR_DIE(sigaddset, "sigaddset failed", &sigs, SIGCHLD);

	SUCCEED_OR_DIE(sigprocmask, "sigprocmask failed", SIG_SETMASK, &sigs,
		       &oldmask);

	/* don't wait on a SIGCHLD */
	SUCCEED_OR_DIE(sigdelset, "sigaddset failed", &sigs, SIGCHLD);

	/* Run a child that will wake us up */
	child = create_sig_proc(0, signo, 1);

	TEST(sigwaitinfo(&sigs, (void *)1, NULL));
	REPORT_SUCCESS(-1, EFAULT);

	SUCCEED_OR_DIE(sigprocmask, "sigprocmask failed", SIG_SETMASK, &oldmask,
		       &oldmask);

	kill(child, SIGTERM);
}

void test_bad_address2(swi_func sigwaitinfo, int signo)
{
	pid_t pid;
	int status;

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "fork() failed");
	case 0:
		signal(SIGSEGV, SIG_DFL);

		/*
		 * depending on glibc implementation we should
		 * either crash or get EFAULT
		 */
		TEST(sigwaitinfo((void *)1, NULL, NULL));

		if (TEST_RETURN == -1 && TEST_ERRNO == EFAULT)
			_exit(0);

		tst_resm(TINFO | TTERRNO, "swi_func returned: %ld",
			TEST_RETURN);
		_exit(1);
		break;
	default:
		break;
	}

	SUCCEED_OR_DIE(waitpid, "waitpid failed", pid, &status, 0);

	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
		tst_resm(TPASS, "Test passed");
		return;
	}

	if (WIFEXITED(status)) {
		tst_resm(TFAIL, "Unrecognised child exit code: %d",
			WEXITSTATUS(status));
	}
	if (WIFSIGNALED(status)) {
		tst_resm(TFAIL, "Unrecognised child termsig: %d",
			WTERMSIG(status));
	}
}

void test_bad_address3(swi_func sigwaitinfo, int signo)
{
	sigset_t sigs;
	SUCCEED_OR_DIE(sigemptyset, "sigemptyset failed", &sigs);

	TEST(sigwaitinfo(&sigs, NULL, (void *)1));
	REPORT_SUCCESS(-1, EFAULT);
}

struct test_desc {
	test_func tf;
	swi_func swi;
	int signo;
} tests[] = {
#ifdef TEST_RT_SIGTIMEDWAIT
	{
	test_empty_set, my_rt_sigtimedwait, SIGUSR1}, {
	test_unmasked_matching, my_rt_sigtimedwait, SIGUSR1}, {
	test_masked_matching, my_rt_sigtimedwait, SIGUSR1}, {
	test_unmasked_matching_noinfo, my_rt_sigtimedwait, SIGUSR1}, {
	test_masked_matching_noinfo, my_rt_sigtimedwait, SIGUSR1}, {
	test_bad_address, my_rt_sigtimedwait, SIGUSR1}, {
	test_bad_address2, my_rt_sigtimedwait, SIGUSR1}, {
	test_bad_address3, my_rt_sigtimedwait, SIGUSR1}, {
	test_timeout, my_rt_sigtimedwait, 0},
	    /* Special cases */
	    /* 1: sigwaitinfo does respond to ignored signal */
	{
	test_masked_matching, my_rt_sigtimedwait, SIGUSR2},
	    /* 2: An ignored signal doesn't cause sigwaitinfo to return EINTR */
	{
	test_timeout, my_rt_sigtimedwait, SIGUSR2},
	    /* 3: The handler is not called when the signal is waited for by sigwaitinfo */
	{
	test_masked_matching, my_rt_sigtimedwait, SIGTERM},
	    /* 4: Simultaneous realtime signals are delivered in the order of increasing signal number */
	{
	test_masked_matching_rt, my_rt_sigtimedwait, -1},
#endif
#if defined TEST_SIGWAIT
	{
	test_unmasked_matching_noinfo, my_sigwait, SIGUSR1}, {
	test_masked_matching_noinfo, my_sigwait, SIGUSR1},
#endif
#if defined TEST_SIGWAITINFO
	{
	test_empty_set, my_sigwaitinfo, SIGUSR1}, {
	test_unmasked_matching, my_sigwaitinfo, SIGUSR1}, {
	test_masked_matching, my_sigwaitinfo, SIGUSR1}, {
	test_unmasked_matching_noinfo, my_sigwaitinfo, SIGUSR1}, {
	test_masked_matching_noinfo, my_sigwaitinfo, SIGUSR1}, {
	test_bad_address, my_sigwaitinfo, SIGUSR1}, {
	test_bad_address2, my_sigwaitinfo, SIGUSR1},
#endif
#if defined TEST_SIGTIMEDWAIT
	{
	test_empty_set, my_sigtimedwait, SIGUSR1}, {
	test_unmasked_matching, my_sigtimedwait, SIGUSR1}, {
	test_masked_matching, my_sigtimedwait, SIGUSR1}, {
	test_unmasked_matching_noinfo, my_sigtimedwait, SIGUSR1}, {
	test_masked_matching_noinfo, my_sigtimedwait, SIGUSR1}, {
	test_bad_address, my_sigtimedwait, SIGUSR1}, {
	test_bad_address2, my_sigtimedwait, SIGUSR1}, {
	test_bad_address3, my_sigtimedwait, SIGUSR1}, {
	test_timeout, my_sigtimedwait, 0},
#endif
};

#if defined TEST_SIGWAITINFO
const char *TCID = "sigwaitinfo01";
#elif defined TEST_RT_SIGTIMEDWAIT
const char *TCID = "rt_sigtimedwait01";
#elif defined TEST_SIGTIMEDWAIT
const char *TCID = "sigtimedwait01";
#elif defined TEST_SIGWAIT
const char *TCID = "sigwait01";
#endif

int TST_TOTAL = ARRAY_SIZE(tests);

int main(int argc, char **argv)
{
	unsigned i;
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		for (i = 0; i < ARRAY_SIZE(tests); i++) {
			alarm(10);	/* arrange a 10 second timeout */
			tst_resm(TINFO, "%p, %d", tests[i].swi, tests[i].signo);
			tests[i].tf(tests[i].swi, tests[i].signo);
		}
		alarm(0);
	}

	cleanup();
	tst_exit();
}
