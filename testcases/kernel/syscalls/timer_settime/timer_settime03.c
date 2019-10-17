// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) HCL Technologies Ltd,.  All Rights Reserved.
 *
 * Author:	Prachi Parekh <prachi.r@hcl.com>
 *
 */
/*
 * This tests the timer_settime(2) syscall under various conditions:
 *
 * 1) Using a periodic timer invoking signal handler
 * 2) Using absolute timer invoking signal handler
 *
 * All of these tests are supposed to be successful.
 */

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "tst_test.h"
#include "lapi/common_timers.h"
#include <stdio.h>
#include "tst_safe_macros.h"

#define SIG SIGALRM

static struct timespec timenow;
static struct itimerspec new_set, old_set;
static kernel_timer_t timer;
static int handler_var = 0;


static struct testcase {
	struct itimerspec	*old_ptr;
	int			it_value_tv_sec;
	int			it_value_tv_nsec;
	int			it_interval_tv_sec;
	int	         	it_interval_tv_nsec;
	int			flag;
	char			*description;
} tcases[] = {
	{&old_set, 0, 5, 0, 5,TIMER_ABSTIME, "using absolute time"},
	{NULL,     0, 5, 0, 5, 0, "using periodic timer"},
};


static void handler(int sig, siginfo_t *si, void *uc)
{
	handler_var = 1;
}


static void run(unsigned int n)
{
	unsigned int i;
	struct testcase *tc = &tcases[n];
	tst_res(TINFO, "n = %d",n);

	struct sigevent evp;
	struct sigaction sa;

	tst_res(TINFO, "Testing for %s:", tc->description);


	for (i = 0; i < CLOCKS_DEFINED; ++i) {
		clock_t clock = clock_list[i];

		tst_res(TINFO, "i= %d:", i);

		/* Establish handler for timer signal */

		tst_res(TINFO, "Establishing handler for siganl %d:",SIG);
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = handler;
		sigemptyset(&sa.sa_mask);
		if (sigaction(SIG, &sa, NULL) == -1)
			continue;


		evp.sigev_value  = (union sigval) 0;
		evp.sigev_signo  = SIG;
		evp.sigev_notify = SIGEV_SIGNAL;


		if (clock == CLOCK_PROCESS_CPUTIME_ID ||
				clock == CLOCK_THREAD_CPUTIME_ID) {
			if (!have_cputime_timers())
				continue;
		}

		TEST(tst_syscall(__NR_timer_create, clock, &evp, &timer));


		if (TST_RET != 0) {
			if (possibly_unsupported(clock) && TST_ERR == EINVAL) {
				tst_res(TPASS | TTERRNO,
						"%s unsupported, failed as expected",
						get_clock_str(clock));
			} else {
				tst_res(TBROK | TTERRNO,
						"timer_create(%s) failed",
						get_clock_str(clock));
			}
			continue;
		}

		memset(&new_set, 0, sizeof(new_set));
		memset(&old_set, 0, sizeof(old_set));

		new_set.it_value.tv_sec = tc->it_value_tv_sec;
		new_set.it_value.tv_nsec = tc->it_value_tv_sec * 1000000;
		new_set.it_interval.tv_sec= tc->it_interval_tv_sec;
		new_set.it_interval.tv_nsec = tc->it_interval_tv_nsec * 1000000;

		if (tc->flag & TIMER_ABSTIME) {
			if (clock_gettime(clock, &timenow) < 0) {
				tst_res(TBROK,
						"clock_gettime(%s) failed - skipping the test",
						get_clock_str(clock));
				continue;
			}
			new_set.it_value.tv_sec += timenow.tv_sec;
		}

		TEST(tst_syscall(__NR_timer_settime, timer,
					tc->flag, &new_set, tc->old_ptr));
		/* sleep for sometime so periodic timer expires in that time*/
		usleep(10000);

		if (handler_var == 0) {
			tst_res(TFAIL | TTERRNO, "%s failed",
					get_clock_str(clock));
		} else {
			tst_res(TPASS, "%s was successful",
					get_clock_str(clock));

			handler_var = 0;
			tst_res(TINFO, "Caught signal %d\n", SIG);
		}
	}
}


static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
