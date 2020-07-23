// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Jiri Palecek<jpalecek@web.de>, 2009 */

#include "libsigwait.h"

static int my_sigwait(const sigset_t * set,
		      siginfo_t * info LTP_ATTRIBUTE_UNUSED,
		      void *timeout LTP_ATTRIBUTE_UNUSED)
{
	int ret;
	int err = sigwait(set, &ret);

	if (err == 0)
		return ret;
	errno = err;
	return -1;
}

struct sigwait_test_desc tests[] = {
	{ test_unmasked_matching_noinfo, SIGUSR1},
	{ test_masked_matching_noinfo, SIGUSR1},
};

static void run(unsigned int i)
{
	struct sigwait_test_desc *tc = &tests[i];

	tc->tf(my_sigwait, tc->signo, TST_LIBC_TIMESPEC);
}

static struct tst_test test = {
	.test= run,
	.tcnt = ARRAY_SIZE(tests),
	.setup = sigwait_setup,
	.forks_child = 1,
};
