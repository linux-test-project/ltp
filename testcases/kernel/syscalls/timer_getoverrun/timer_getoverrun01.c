// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Porting from Crackerjack to LTP is done by:
 *              Manas Kumar Nayak <maknayak@in.ibm.com>
 *
 * Copyright (c) Linux Test Project, 2009-2023
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test checks base timer_getoverrun() functionality.
 */

#include <signal.h>
#include <time.h>
#include "tst_safe_clocks.h"
#include "lapi/syscalls.h"
#include "lapi/common_timers.h"

static void run(void)
{
	kernel_timer_t timer;
	struct sigevent ev;

	ev.sigev_value = (union sigval) 0;
	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGALRM;

	if (tst_syscall(__NR_timer_create, CLOCK_REALTIME, &ev, &timer))
		tst_brk(TBROK | TERRNO, "timer_create() failed");

	TST_EXP_POSITIVE(tst_syscall(__NR_timer_getoverrun, timer));

	if (tst_syscall(__NR_timer_delete, timer))
		tst_brk(TBROK | TERRNO, "timer_delete() failed");

	TST_EXP_FAIL(tst_syscall(__NR_timer_getoverrun, timer), EINVAL);
}

static struct tst_test test = {
	.test_all = run,
};
