// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Jiri Palecek<jpalecek@web.de>, 2009 */

#include "tst_test.h"
#include "tst_timer.h"
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include "lapi/syscalls.h"
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

static void empty_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

typedef int (*swi_func) (const sigset_t * set, siginfo_t * info,
			 void * timeout);
typedef void (*test_func) (swi_func, int, enum tst_ts_type type);

#ifdef TEST_SIGWAIT
static int my_sigwait(const sigset_t * set, siginfo_t * info,
		      void *timeout)
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
			  void *timeout LTP_ATTRIBUTE_UNUSED)
{
	return sigwaitinfo(set, info);
}
#endif

#ifdef TEST_SIGTIMEDWAIT
static int my_sigtimedwait(const sigset_t * set, siginfo_t * info,
			   void *timeout)
{
	return sigtimedwait(set, info, timeout);
}
#endif

#ifdef TEST_RT_SIGTIMEDWAIT
static int my_rt_sigtimedwait(const sigset_t * set, siginfo_t * info,
			      void *timeout)
{
	/* _NSIG is always the right number of bits of signal map for all arches */
	return tst_syscall(__NR_rt_sigtimedwait, set, info, timeout, _NSIG/8);
}

#if (__NR_rt_sigtimedwait_time64 != __LTP__NR_INVALID_SYSCALL)
static int my_rt_sigtimedwait_time64(const sigset_t * set, siginfo_t * info,
				     void *timeout)
{
	/* _NSIG is always the right number of bits of signal map for all arches */
	return tst_syscall(__NR_rt_sigtimedwait_time64, set, info, timeout, _NSIG/8);
}
#endif
#endif

void test_empty_set(swi_func sigwaitinfo, int signo,
		    enum tst_ts_type type LTP_ATTRIBUTE_UNUSED)
{
	sigset_t sigs;
	siginfo_t si;
	pid_t child;

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

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

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

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

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

	TEST(sigaddset(&sigs, signo));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

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

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

	TEST(sigaddset(&sigs, signo));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

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

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

	TEST(sigaddset(&sigs, signo));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	/* let's not get interrupted by our dying child */
	TEST(sigaddset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	TEST(sigdelset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigdelset() failed");

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

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

	TEST(sigaddset(&sigs, signo));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	TEST(sigaddset(&sigs, signo + 1));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	/* let's not get interrupted by our dying child */
	TEST(sigaddset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	TEST(sigdelset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigdelset() failed");

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

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

	TEST(sigaddset(&sigs, signo));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	/* let's not get interrupted by our dying child */
	TEST(sigaddset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	TEST(sigdelset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigdelset() failed");

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

	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

	TEST(sigaddset(&sigs, signo));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	/* let's not get interrupted by our dying child */
	TEST(sigaddset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigaddset() failed");

	TEST(sigprocmask(SIG_SETMASK, &sigs, &oldmask));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigprocmask() failed");

	/* don't wait on a SIGCHLD */
	TEST(sigdelset(&sigs, SIGCHLD));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigdelset() failed");

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

	switch (pid = fork()) {
	case -1:
		tst_brk(TBROK | TERRNO, "fork() failed");
	case 0:
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
		break;
	default:
		break;
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
	TEST(sigemptyset(&sigs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "sigemptyset() failed");

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
	test_empty_set, NULL, SIGUSR1}, {
	test_unmasked_matching, NULL, SIGUSR1}, {
	test_masked_matching, NULL, SIGUSR1}, {
	test_unmasked_matching_noinfo, NULL, SIGUSR1}, {
	test_masked_matching_noinfo, NULL, SIGUSR1}, {
	test_bad_address, NULL, SIGUSR1}, {
	test_bad_address2, NULL, SIGUSR1}, {
	test_bad_address3, NULL, SIGUSR1}, {
	test_timeout, NULL, 0},
	    /* Special cases */
	    /* 1: sigwaitinfo does respond to ignored signal */
	{
	test_masked_matching, NULL, SIGUSR2},
	    /* 2: An ignored signal doesn't cause sigwaitinfo to return EINTR */
	{
	test_timeout, NULL, SIGUSR2},
	    /* 3: The handler is not called when the signal is waited for by sigwaitinfo */
	{
	test_masked_matching, NULL, SIGTERM},
	    /* 4: Simultaneous realtime signals are delivered in the order of increasing signal number */
	{
	test_masked_matching_rt, NULL, -1},
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

static struct test_variants {
	swi_func swi;
	enum tst_ts_type type;
	char *desc;
} variants[] = {
#ifdef TEST_RT_SIGTIMEDWAIT

#if (__NR_rt_sigtimedwait != __LTP__NR_INVALID_SYSCALL)
	{ .swi = my_rt_sigtimedwait, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_rt_sigtimedwait_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .swi = my_rt_sigtimedwait_time64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif

#else /* !TEST_RT_SIGTIMEDWAIT */

	{ .swi = NULL, .type = TST_LIBC_TIMESPEC, .desc = "syscall with libc spec"},

#endif /* TEST_RT_SIGTIMEDWAIT */
};

static void run(unsigned int i)
{
	struct test_variants *tv = &variants[tst_variant];
	struct test_desc *tc = &tests[i];
	swi_func swi;

	swi = tv->swi ? tv->swi : tc->swi;

	tc->tf(swi, tc->signo, tv->type);
}

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);

	signal(SIGUSR1, empty_handler);
	signal(SIGALRM, empty_handler);
	signal(SIGUSR2, SIG_IGN);

	alarm(10);	/* arrange a 10 second timeout */
}

static void cleanup(void)
{
	alarm(0);
}

static struct tst_test test = {
	.test= run,
	.tcnt = ARRAY_SIZE(tests),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
