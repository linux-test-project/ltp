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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
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

Usage is: ./performance_counter02 [-c num-hw-counters] [-v]

Use -c N if you have more than 8 hardware counters.  The -v flag makes
it print out the values of each counter.
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

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#define PR_TASK_PERF_COUNTERS_DISABLE           31
#define PR_TASK_PERF_COUNTERS_ENABLE            32

/* Global Variables */
char *TCID     = "performance_counter02"; /* test program identifier.          */
int  TST_TOTAL = 1;                  /* total number of tests in this file.   */

typedef unsigned int u32;
typedef unsigned long long u64;
typedef long long s64;

struct perf_counter_hw_event {
        s64                     type;
        u64                     irq_period;
        u32                     record_type;

        u32                     disabled     :  1, /* off by default */
                                nmi          :  1, /* NMI sampling   */
                                raw          :  1, /* raw event type */
                                __reserved_1 : 29;
        u64                     __reserved_2;
};

enum hw_event_types {
	PERF_COUNT_CYCLES		=  0,
	PERF_COUNT_INSTRUCTIONS		=  1,
	PERF_COUNT_CACHE_REFERENCES	=  2,
	PERF_COUNT_CACHE_MISSES		=  3,
	PERF_COUNT_BRANCH_INSTRUCTIONS	=  4,
	PERF_COUNT_BRANCH_MISSES	=  5,

	/*
	 * Special "software" counters provided by the kernel, even if
	 * the hardware does not support performance counters. These
	 * counters measure various physical and sw events of the
	 * kernel (and allow the profiling of them as well):
	 */
	PERF_COUNT_CPU_CLOCK		= -1,
	PERF_COUNT_TASK_CLOCK		= -2,
	/*
	 * Future software events:
	 */
	/* PERF_COUNT_PAGE_FAULTS	= -3,
	   PERF_COUNT_CONTEXT_SWITCHES	= -4, */
};

int sys_perf_counter_open(struct perf_counter_hw_event *hw_event,
			  pid_t pid, int cpu, int group_fd, unsigned long flags)
{
	return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd,
		       flags);
}

#define MAX_CTRS	50
#define LOOPS	1000000000

void do_work(void)
{
	int i;

	for (i = 0; i < LOOPS; ++i)
		asm volatile("" : : "g" (i));
}

void cleanup(void) { /* Stub function. */ }

int
main(int ac, char **av)
{
	int tsk0;
	int hwfd[MAX_CTRS], tskfd[MAX_CTRS];
	struct perf_counter_hw_event tsk_event;
	struct perf_counter_hw_event hw_event;
	unsigned long long vt0, vt[MAX_CTRS], vh[MAX_CTRS], vtsum, vhsum;
	int i, n, nhw;
	int verbose = 0;
	double ratio;

	nhw = 8;
	while ((i = getopt(ac, av, "c:v")) != -1) {
		switch (i) {
		case 'c':
			n = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
			fprintf(stderr, "Usage: %s [-c #hwctrs] [-v]\n", av[0]);
			exit(1);
		}
	}

	if (nhw < 0 || nhw > MAX_CTRS - 4) {
		fprintf(stderr, "invalid number of hw counters specified: %d\n",
			nhw);
		exit(1);
	}

	n = nhw + 4;

	memset(&tsk_event, 0, sizeof(tsk_event));
	tsk_event.type = PERF_COUNT_TASK_CLOCK;
	tsk_event.disabled = 1;

	memset(&hw_event, 0, sizeof(hw_event));
	hw_event.disabled = 1;
	hw_event.type = PERF_COUNT_INSTRUCTIONS;

	tsk0 = sys_perf_counter_open(&tsk_event, 0, -1, -1, 0);
	if (tsk0 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "perf_counter_open failed (1)");
	} else {

		tsk_event.disabled = 0;
		for (i = 0; i < n; ++i) {
			hwfd[i] =  sys_perf_counter_open(&hw_event, 0, -1,
							 -1, 0);
			tskfd[i] = sys_perf_counter_open(&tsk_event, 0, -1,
							 hwfd[i], 0);
			if (tskfd[i] == -1 || hwfd[i] == -1) {
				tst_brkm(TBROK | TERRNO, cleanup,
					"perf_counter_open failed (2)");
			}
		}
	}

	prctl(PR_TASK_PERF_COUNTERS_ENABLE);
	do_work();
	prctl(PR_TASK_PERF_COUNTERS_DISABLE);

	if (read(tsk0, &vt0, sizeof(vt0)) != sizeof(vt0)) {
		tst_brkm(TBROK | TERRNO, cleanup,
			"error reading task clock counter");
	}

	vtsum = vhsum = 0;
	for (i = 0; i < n; ++i) {
		if (read(tskfd[i], &vt[i], sizeof(vt[i])) != sizeof(vt[i]) ||
		    read(hwfd[i], &vh[i], sizeof(vh[i])) != sizeof(vh[i])) {
			tst_brkm(TBROK | TERRNO, cleanup,
				"error reading counter(s)");
		}
		vtsum += vt[i];
		vhsum += vh[i];
	}

	tst_resm(TINFO, "overall task clock: %lld", vt0);
	tst_resm(TINFO, "hw sum: %lld, task clock sum: %lld", vhsum, vtsum);
	if (verbose) {
		printf("hw counters:");
		for (i = 0; i < n; ++i)
			printf(" %lld", vh[i]);
		printf("\ntask clock counters:");
		for (i = 0; i < n; ++i)
			printf(" %lld", vt[i]);
		printf("\n");
	}
	ratio = (double)vtsum / vt0;
	tst_resm(TINFO, "ratio: %.2f", ratio);
	if (ratio > nhw + 0.0001) {
		tst_resm(TFAIL, "test failed (ratio was greater than )");
	} else {
		tst_resm(TINFO, "test passed");
	}
	tst_exit();
}
