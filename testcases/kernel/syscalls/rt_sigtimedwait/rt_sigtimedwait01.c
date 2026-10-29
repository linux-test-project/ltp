// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Jiri Palecek<jpalecek@web.de>, 2009 */

#include "time64_variants.h"
#include "tse_sigwait.h"

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

struct sigwait_test_desc tests[] = {
	{ tse_empty_set, SIGUSR1},
	{ tse_unmasked_matching, SIGUSR1},
	{ tse_masked_matching, SIGUSR1},
	{ tse_unmasked_matching_noinfo, SIGUSR1},
	{ tse_masked_matching_noinfo, SIGUSR1},
	{ tse_bad_address, SIGUSR1},
	{ tse_bad_address2, SIGUSR1},
	{ tse_bad_address3, SIGUSR1},
	{ tse_timeout, 0},
	/* Special cases */
	/* 1: sigwaitinfo does respond to ignored signal */
	{ tse_masked_matching, SIGUSR2},
	/* 2: An ignored signal doesn't cause sigwaitinfo to return EINTR */
	{ tse_timeout, SIGUSR2},
	/* 3: The handler is not called when the signal is waited for by sigwaitinfo */
	{ tse_masked_matching, SIGTERM},
	/* 4: Simultaneous realtime signals are delivered in the order of increasing signal number */
	{ tse_masked_matching_rt, -1},
};

static struct time64_variants variants[] = {
#if (__NR_rt_sigtimedwait != __LTP__NR_INVALID_SYSCALL)
	{ .sigwait = my_rt_sigtimedwait, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_rt_sigtimedwait_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .sigwait = my_rt_sigtimedwait_time64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void run(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct sigwait_test_desc *tc = &tests[i];

	tc->tf(tv->sigwait, tc->signo, tv->ts_type);
}

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
	tse_sigwait_setup();
}

static struct tst_test test = {
	.test= run,
	.tcnt = ARRAY_SIZE(tests),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.forks_child = 1,
};
