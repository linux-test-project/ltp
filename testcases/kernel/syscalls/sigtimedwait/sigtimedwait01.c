// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Jiri Palecek<jpalecek@web.de>, 2009 */

#include "tse_sigwait.h"

static int my_sigtimedwait(const sigset_t * set, siginfo_t * info,
			   void *timeout)
{
	return sigtimedwait(set, info, timeout);
}

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
};

static void run(unsigned int i)
{
	struct sigwait_test_desc *tc = &tests[i];

	tc->tf(my_sigtimedwait, tc->signo, TST_LIBC_TIMESPEC);
}

static struct tst_test test = {
	.test= run,
	.tcnt = ARRAY_SIZE(tests),
	.setup = tse_sigwait_setup,
	.forks_child = 1,
};
