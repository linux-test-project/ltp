/******************************************************************************/
/*                                                                            */
/* Paul Mackerras <paulus@samba.org>, 2009                                    */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/*
Here's a little test program that checks whether software counters
(specifically, the task clock counter) work correctly when they're in
a group with hardware counters.

What it does is to create several groups, each with one hardware
counter, counting instructions, plus a task clock counter.  It needs
to know an upper bound N on the number of hardware counters you have
(N defaults to 8), and it creates N+4 groups to force them to be
multiplexed.  It also creates an overall task clock counter.

Then it spins for a while, and then stops all the counters and reads
them.  It takes the total of the task clock counters in the groups and
computes the ratio of that total to the overall execution time from
the overall task clock counter.

That ratio should be equal to the number of actual hardware counters
that can count instructions.  If the task clock counters in the groups
don't stop when their group gets taken off the PMU, the ratio will
instead be close to N+4.  The program will declare that the test fails
if the ratio is greater than N (actually, N + 0.0001 to allow for FP
rounding errors).

Could someone run this on x86 on the latest PCL tree and let me know
what happens?  I don't have an x86 crash box easily to hand.  On
powerpc, it passes, but I think that is because I am missing setting
counter->prev_count in arch/powerpc/kernel/perf_counter.c, and I think
that means that enabling/disabling a group with a task clock counter
in it won't work correctly (I'll do a test program for that next).

Usage is: ./performance_counter02  [-v]

The -v flag makes it print out the values of each counter.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"
#include <sys/prctl.h>
#include <sys/types.h>
#include <linux/types.h>

#if HAVE_PERF_EVENT_ATTR
# include <linux/perf_event.h>
#endif

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

char *TCID = "perf_event_open02";
int TST_TOTAL = 1;

#if HAVE_PERF_EVENT_ATTR

#define MAX_CTRS	1000
#define LOOPS		100000000

static int count_hardware_counters(void);
static void setup(void);
static void verify(void);
static void cleanup(void);
static void help(void);

static int n, nhw;
static int verbose;
static option_t options[] = {
	{"v", &verbose, NULL},
	{NULL, NULL, NULL},
};

static int tsk0;
static int hwfd[MAX_CTRS], tskfd[MAX_CTRS];

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, options, help);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		verify();
	}

	cleanup();
	tst_exit();
}

static int perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
	int cpu, int group_fd, unsigned long flags)
{
	int ret;

	ret = ltp_syscall(__NR_perf_event_open, hw_event, pid, cpu,
			  group_fd, flags);
	return ret;
}


static void do_work(void)
{
	int i;

	for (i = 0; i < LOOPS; ++i)
		asm volatile (""::"g" (i));
}

struct read_format {
	unsigned long long value;
	/* if PERF_FORMAT_TOTAL_TIME_ENABLED */
	unsigned long long time_enabled;
	/* if PERF_FORMAT_TOTAL_TIME_RUNNING */
	unsigned long long time_running;
};

static int count_hardware_counters(void)
{
	struct perf_event_attr hw_event;
	int i, hwctrs = 0;
	int fdarry[MAX_CTRS];
	struct read_format buf;

	memset(&hw_event, 0, sizeof(struct perf_event_attr));

	hw_event.type = PERF_TYPE_HARDWARE;
	hw_event.size = sizeof(struct perf_event_attr);
	hw_event.disabled = 1;
	hw_event.config =  PERF_COUNT_HW_INSTRUCTIONS;
	hw_event.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED |
		PERF_FORMAT_TOTAL_TIME_RUNNING;

	for (i = 0; i < MAX_CTRS; i++) {
		fdarry[i] = perf_event_open(&hw_event, 0, -1, -1, 0);
		if (fdarry[i] == -1) {
			if (errno == ENOENT || errno == ENODEV) {
				tst_brkm(TCONF | TERRNO, cleanup,
				         "PERF_COUNT_HW_INSTRUCTIONS not supported");
			}
			tst_brkm(TBROK | TERRNO, cleanup,
				 "perf_event_open failed at iteration:%d", i);
		}

		if (prctl(PR_TASK_PERF_EVENTS_ENABLE) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "prctl(PR_TASK_PERF_EVENTS_ENABLE) failed");
		}

		do_work();

		if (prctl(PR_TASK_PERF_EVENTS_DISABLE) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "prctl(PR_TASK_PERF_EVENTS_DISABLE) failed");
		}

		if (read(fdarry[i], &buf, sizeof(buf)) != sizeof(buf)) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "error reading counter(s)");
		}

		if (verbose == 1) {
			printf("at iteration:%d value:%lld time_enabled:%lld "
			       "time_running:%lld\n", i, buf.value,
			       buf.time_enabled, buf.time_running);
		}

		/*
		 * Normally time_enabled and time_running are the same value.
		 * But if more events are started than available counter slots
		 * on the PMU, then multiplexing happens and events run only
		 * part of the time. Time_enabled and time_running's values
		 * will be different. In this case the time_enabled and time_
		 * running values can be used to scale an estimated value for
		 * the count. So if buf.time_enabled and buf.time_running are
		 * not equal, we can think that PMU hardware counters
		 * multiplexing happens and the number of the opened events
		 * are the number of max available hardware counters.
		 */
		if (buf.time_enabled != buf.time_running) {
			hwctrs = i;
			break;
		}
	}

	for (i = 0; i <= hwctrs; i++)
		SAFE_CLOSE(cleanup, fdarry[i]);

	return hwctrs;
}

static void setup(void)
{
	int i;
	struct perf_event_attr tsk_event, hw_event;

	/*
	 * According to perf_event_open's manpage, the official way of
	 * knowing if perf_event_open() support is enabled is checking for
	 * the existence of the file /proc/sys/kernel/perf_event_paranoid.
	 */
	if (access("/proc/sys/kernel/perf_event_paranoid", F_OK) == -1)
		tst_brkm(TCONF, NULL, "Kernel doesn't have perf_event support");

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	nhw = count_hardware_counters();
	n = nhw + 4;

	memset(&hw_event, 0, sizeof(struct perf_event_attr));
	memset(&tsk_event, 0, sizeof(struct perf_event_attr));

	tsk_event.type =  PERF_TYPE_SOFTWARE;
	tsk_event.size = sizeof(struct perf_event_attr);
	tsk_event.disabled = 1;
	tsk_event.config = PERF_COUNT_SW_TASK_CLOCK;

	hw_event.type = PERF_TYPE_HARDWARE;
	hw_event.size = sizeof(struct perf_event_attr);
	hw_event.disabled = 1;
	hw_event.config =  PERF_COUNT_HW_INSTRUCTIONS;

	tsk0 = perf_event_open(&tsk_event, 0, -1, -1, 0);
	if (tsk0 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "perf_event_open failed");
	} else {
		tsk_event.disabled = 0;
		for (i = 0; i < n; ++i) {
			hwfd[i] = perf_event_open(&hw_event, 0, -1, -1, 0);
			tskfd[i] = perf_event_open(&tsk_event, 0, -1,
						   hwfd[i], 0);
			if (tskfd[i] == -1 || hwfd[i] == -1) {
				tst_brkm(TBROK | TERRNO, cleanup,
					 "perf_event_open failed");
			}
		}
	}
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < n; i++) {
		if (hwfd[i] > 0 && close(hwfd[i]) == -1)
			tst_resm(TWARN | TERRNO, "close(%d) failed", hwfd[i]);
		if (tskfd[i] > 0 && close(tskfd[i]) == -1)
			tst_resm(TWARN | TERRNO, "close(%d) failed", tskfd[i]);
	}

	if (tsk0 > 0 && close(tsk0) == -1)
		tst_resm(TWARN | TERRNO, "close(%d) failed", tsk0);
}

static void verify(void)
{
	unsigned long long vt0, vt[MAX_CTRS], vh[MAX_CTRS];
	unsigned long long vtsum = 0, vhsum = 0;
	int i;
	double ratio;

	if (prctl(PR_TASK_PERF_EVENTS_ENABLE) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "prctl(PR_TASK_PERF_EVENTS_ENABLE) failed");
	}

	do_work();

	if (prctl(PR_TASK_PERF_EVENTS_DISABLE) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "prctl(PR_TASK_PERF_EVENTS_DISABLE) failed");
	}

	if (read(tsk0, &vt0, sizeof(vt0)) != sizeof(vt0)) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "error reading task clock counter");
	}

	for (i = 0; i < n; ++i) {
		if (read(tskfd[i], &vt[i], sizeof(vt[i])) != sizeof(vt[i]) ||
		    read(hwfd[i], &vh[i], sizeof(vh[i])) != sizeof(vh[i])) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "error reading counter(s)");
		}
		vtsum += vt[i];
		vhsum += vh[i];
	}

	tst_resm(TINFO, "overall task clock: %llu", vt0);
	tst_resm(TINFO, "hw sum: %llu, task clock sum: %llu", vhsum, vtsum);

	if (verbose == 1) {
		printf("hw counters:");
		for (i = 0; i < n; ++i)
			printf(" %llu", vh[i]);
		printf("\ntask clock counters:");
		for (i = 0; i < n; ++i)
			printf(" %llu", vt[i]);
		printf("\n");
	}

	ratio = (double)vtsum / vt0;
	tst_resm(TINFO, "ratio: %lf", ratio);
	if (ratio > nhw + 0.0001) {
		tst_resm(TFAIL, "test failed (ratio was greater than )");
	} else {
		tst_resm(TPASS, "test passed");
	}
}

static void help(void)
{
	printf("  -v      Print verbose information\n");
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "This system doesn't have "
		 "header file:<linux/perf_event.h> or "
		 "no struct perf_event_attr defined");
}
#endif
