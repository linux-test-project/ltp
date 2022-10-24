// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 03/2001 - Written by Wayne Boyer
 *
 */

/*\
 * [Description]
 *
 * Check that a setitimer() call pass with timer seting.
 * Check if signal is generated correctly when timer expiration.
 */

#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define USEC1	10000
#define USEC2	20000

static struct itimerval *value, *ovalue;

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
	struct tcase *tc = &tcases[i];

	pid = SAFE_FORK();

	if (pid == 0) {
		tst_res(TINFO, "tc->which = %s", tc->des);

		tst_no_corefile(0);

		set_setitimer_value(USEC1, 0);
		TST_EXP_PASS(sys_setitimer(tc->which, value, NULL));

		set_setitimer_value(USEC2, USEC2);
		TST_EXP_PASS(sys_setitimer(tc->which, value, ovalue));

		if (ovalue->it_value.tv_sec != 0 || ovalue->it_value.tv_usec >= USEC2)
			tst_brk(TFAIL, "old timer value is not within the expected range");

		for (;;)
			;
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == tc->signo)
		tst_res(TPASS, "Child received signal: %s", tst_strsig(tc->signo));
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(status));
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.test = verify_setitimer,
	.bufs = (struct tst_buffers[]) {
		{&value,  .size = sizeof(struct itimerval)},
		{&ovalue, .size = sizeof(struct itimerval)},
		{}
	}
};
