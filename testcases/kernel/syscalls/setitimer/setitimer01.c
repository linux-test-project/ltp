// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2002-2022
 * Copyright (c) International Business Machines Corp., 2001
 * 03/2001 - Written by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Spawn a child, verify that setitimer() syscall passes and it ends up
 * counting inside expected boundaries. Then verify from the parent that
 * the syscall sent the correct signal to the child.
 */

#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "tst_safe_clocks.h"

static struct timeval tv;
static struct itimerval *value, *ovalue;
static volatile unsigned long sigcnt;
static long time_step;
static long time_sec;
static long time_usec;

static struct tcase {
	int which;
	char *des;
	int signo;
} tcases[] = {
	{ITIMER_REAL,    "ITIMER_REAL",    SIGALRM},
	{ITIMER_VIRTUAL, "ITIMER_VIRTUAL", SIGVTALRM},
	{ITIMER_PROF,    "ITIMER_PROF",    SIGPROF},
};

static int sys_setitimer(int which, void *new_value, void *old_value)
{
	return tst_syscall(__NR_setitimer, which, new_value, old_value);
}

static void sig_routine(int signo LTP_ATTRIBUTE_UNUSED)
{
	sigcnt++;
}

static void set_setitimer_value(int sec, int usec)
{
	value->it_value.tv_sec = sec;
	value->it_value.tv_usec = usec;
	value->it_interval.tv_sec = sec;
	value->it_interval.tv_usec = usec;
}

static void verify_setitimer(unsigned int i)
{
	pid_t pid;
	int status;
	long margin;
	struct tcase *tc = &tcases[i];

	tst_res(TINFO, "tc->which = %s", tc->des);

	if (tc->which == ITIMER_REAL) {
		if (gettimeofday(&tv, NULL) == -1)
			tst_brk(TBROK | TERRNO, "gettimeofday(&tv1, NULL) failed");
		else
			tst_res(TINFO, "Test begin time: %ld.%lds", tv.tv_sec, tv.tv_usec);
	}

	pid = SAFE_FORK();

	if (pid == 0) {
		tst_no_corefile(0);

		set_setitimer_value(time_sec, time_usec);
		TST_EXP_PASS(sys_setitimer(tc->which, value, NULL));

		set_setitimer_value(5 * time_sec, 7 * time_usec);
		TST_EXP_PASS(sys_setitimer(tc->which, value, ovalue));

		TST_EXP_EQ_LI(ovalue->it_interval.tv_sec, time_sec);
		TST_EXP_EQ_LI(ovalue->it_interval.tv_usec, time_usec);

		tst_res(TINFO, "ovalue->it_value.tv_sec=%ld, ovalue->it_value.tv_usec=%ld",
			ovalue->it_value.tv_sec, ovalue->it_value.tv_usec);

		/*
		 * ITIMER_VIRTUAL and ITIMER_PROF timers always expire a
		 * time_step afterward the elapsed time to make sure that
		 * at least counters take effect.
		 */
		margin = tc->which == ITIMER_REAL ? 0 : time_step;

		if (ovalue->it_value.tv_sec == time_sec) {
			if (ovalue->it_value.tv_usec < 0 ||
					ovalue->it_value.tv_usec > time_usec + margin)
				tst_res(TFAIL, "ovalue->it_value.tv_usec is out of range: %ld",
					ovalue->it_value.tv_usec);
		} else {
			if (ovalue->it_value.tv_sec < 0 ||
					ovalue->it_value.tv_sec > time_sec)
				tst_res(TFAIL, "ovalue->it_value.tv_sec is out of range: %ld",
					ovalue->it_value.tv_sec);
		}

		SAFE_SIGNAL(tc->signo, sig_routine);

		set_setitimer_value(0, time_usec);
		TST_EXP_PASS(sys_setitimer(tc->which, value, NULL));

		while (sigcnt <= 10UL)
			;

		SAFE_SIGNAL(tc->signo, SIG_DFL);

		while (1)
			;
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == tc->signo)
		tst_res(TPASS, "Child received signal: %s", tst_strsig(tc->signo));
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(status));

	if (tc->which == ITIMER_REAL) {
		if (gettimeofday(&tv, NULL) == -1)
			tst_brk(TBROK | TERRNO, "gettimeofday(&tv1, NULL) failed");
		else
			tst_res(TINFO, "Test end time: %ld.%lds", tv.tv_sec, tv.tv_usec);
	}
}

static void setup(void)
{
	struct timespec time_res;

	/*
	 * Use CLOCK_MONOTONIC_COARSE resolution for all timers, since its value is
	 * bigger than CLOCK_MONOTONIC and therefore can used for both realtime and
	 * virtual/prof timers resolutions.
	 */
	SAFE_CLOCK_GETRES(CLOCK_MONOTONIC_COARSE, &time_res);

	time_step = time_res.tv_nsec / 1000;
	if (time_step <= 0)
		time_step = 1000;

	tst_res(TINFO, "clock low-resolution: %luns, time step: %luus",
		time_res.tv_nsec, time_step);

	time_sec  = 9 + time_step / 1000;
	time_usec = 3 * time_step;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.setup = setup,
	.test = verify_setitimer,
	.bufs = (struct tst_buffers[]) {
		{&value,  .size = sizeof(struct itimerval)},
		{&ovalue, .size = sizeof(struct itimerval)},
		{}
	}
};
