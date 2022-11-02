// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2002-2022
 * Copyright (c) International Business Machines Corp., 2001
 * 03/2001 - Written by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Spawn a child and verify that setitimer() syscall passes, and it ends up
 * counting inside expected boundaries. Then verify from the parent that our
 * syscall sent the correct signal to the child.
 */

#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "tst_safe_clocks.h"

static struct itimerval *value, *ovalue;
static unsigned long time_step;

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

static void set_setitimer_value(int usec, int o_usec)
{
	value->it_value.tv_sec = 0;
	value->it_value.tv_usec = usec;
	value->it_interval.tv_sec = 0;
	value->it_interval.tv_usec = 0;

	ovalue->it_value.tv_sec = o_usec;
	ovalue->it_value.tv_usec = o_usec;
	ovalue->it_interval.tv_sec = 0;
	ovalue->it_interval.tv_usec = 0;
}

static void verify_setitimer(unsigned int i)
{
	pid_t pid;
	int status;
	int usec = 3 * time_step;
	struct tcase *tc = &tcases[i];

	pid = SAFE_FORK();

	if (pid == 0) {
		tst_res(TINFO, "tc->which = %s", tc->des);

		tst_no_corefile(0);

		set_setitimer_value(usec, 0);
		TST_EXP_PASS(sys_setitimer(tc->which, value, NULL));

		set_setitimer_value(5 * time_step, 7 * time_step);
		TST_EXP_PASS(sys_setitimer(tc->which, value, ovalue));

		tst_res(TINFO, "tv_sec=%ld, tv_usec=%ld",
			ovalue->it_value.tv_sec,
			ovalue->it_value.tv_usec);

		if (ovalue->it_value.tv_sec != 0 || ovalue->it_value.tv_usec > usec)
			tst_res(TFAIL, "Ending counters are out of range");

		for (;;)
			;
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == tc->signo)
		tst_res(TPASS, "Child received signal: %s", tst_strsig(tc->signo));
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(status));
}

static void setup(void)
{
	struct timespec res;

	SAFE_CLOCK_GETRES(CLOCK_MONOTONIC, &res);

	time_step = res.tv_nsec / 1000;
	if (time_step < 10000)
		time_step = 10000;

	tst_res(TINFO, "clock resolution: %luns, time step: %luus",
		res.tv_nsec,
		time_step);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.test = verify_setitimer,
	.setup = setup,
	.bufs = (struct tst_buffers[]) {
		{&value,  .size = sizeof(struct itimerval)},
		{&ovalue, .size = sizeof(struct itimerval)},
		{}
	}
};
