// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2009 Paul Mackerras <paulus@samba.org>
 * Copyright (c) 2014-2022 Linux Test Project
 */
/*
 * Here's a little test program that checks whether software counters
 * (specifically, the task clock counter) work correctly when they're in
 * a group with hardware counters.
 *
 * What it does is to create several groups, each with one hardware
 * counter, counting instructions, plus a task clock counter.  It needs
 * to know an upper bound N on the number of hardware counters you have
 * (N defaults to 8), and it creates N+4 groups to force them to be
 * multiplexed.  It also creates an overall task clock counter.
 *
 * Then it spins for a while, and then stops all the counters and reads
 * them.  It takes the total of the task clock counters in the groups and
 * computes the ratio of that total to the overall execution time from
 * the overall task clock counter.
 *
 * That ratio should be equal to the number of actual hardware counters
 * that can count instructions.  If the task clock counters in the groups
 * don't stop when their group gets taken off the PMU, the ratio will
 * instead be close to N+4.  The program will declare that the test fails
 * if the ratio is greater than N (actually, N + 0.0001 to allow for FP
 * rounding errors).
 */

#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "config.h"
#include "tst_test.h"
#include "lapi/cpuset.h"
#include "lapi/syscalls.h"

#include "perf_event_open.h"

#define MAX_CTRS	1000

struct read_format {
	unsigned long long value;
	/* if PERF_FORMAT_TOTAL_TIME_ENABLED */
	unsigned long long time_enabled;
	/* if PERF_FORMAT_TOTAL_TIME_RUNNING */
	unsigned long long time_running;
};

static char *verbose;

static int ntotal, nhw;
static int tsk0 = -1, hwfd[MAX_CTRS], tskfd[MAX_CTRS];
static int volatile work_done;
static unsigned int est_loops;

static void all_counters_set(int state)
{
	if (prctl(state) == -1)
		tst_brk(TBROK | TERRNO, "prctl(%d) failed", state);
}

static void alarm_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	work_done = 1;
}

static void bench_work(int time_ms)
{
	unsigned int i;
	struct itimerval val;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = alarm_handler;
	sa.sa_flags = SA_RESETHAND;
	SAFE_SIGACTION(SIGALRM, &sa, NULL);

	work_done = 0;
	memset(&val, 0, sizeof(val));
	val.it_value.tv_sec = time_ms / 1000;
	val.it_value.tv_usec = (time_ms % 1000) * 1000;

	if (setitimer(ITIMER_REAL, &val, NULL))
		tst_brk(TBROK | TERRNO, "setitimer");

	while (!work_done) {
		for (i = 0; i < 100000; ++i)
			asm volatile (""::"g" (i));
		est_loops++;
	}

	tst_res(TINFO, "bench_work estimated loops = %u in %d ms", est_loops, time_ms);
}

static void do_work(int mult)
{
	unsigned long i, j, loops = mult * est_loops;

	for (j = 0; j < loops; j++)
		for (i = 0; i < 100000; i++)
			asm volatile (""::"g" (i));
}

#ifndef __s390__
static int count_hardware_counters(void)
{
	struct perf_event_attr hw_event;
	int i, hwctrs = 0;
	int fdarry[MAX_CTRS];
	struct read_format buf, buf2, diff;

	memset(&hw_event, 0, sizeof(struct perf_event_attr));

	hw_event.type = PERF_TYPE_HARDWARE;
	hw_event.size = sizeof(struct perf_event_attr);
	hw_event.disabled = 1;
	hw_event.config =  PERF_COUNT_HW_INSTRUCTIONS;
	hw_event.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED |
		PERF_FORMAT_TOTAL_TIME_RUNNING;

	for (i = 0; i < MAX_CTRS; i++) {
		fdarry[i] = perf_event_open(&hw_event, 0, -1, -1, 0);

		all_counters_set(PR_TASK_PERF_EVENTS_ENABLE);
		do_work(1);
		if (read(fdarry[i], &buf, sizeof(buf)) != sizeof(buf))
			tst_brk(TBROK | TERRNO, "error reading counter(s) #1");
		do_work(1);
		all_counters_set(PR_TASK_PERF_EVENTS_DISABLE);
		if (read(fdarry[i], &buf2, sizeof(buf2)) != sizeof(buf2))
			tst_brk(TBROK | TERRNO, "error reading counter(s) #2");

		diff.value = buf2.value - buf.value;
		diff.time_enabled = buf2.time_enabled - buf.time_enabled;
		diff.time_running = buf2.time_running - buf.time_running;

		tst_res(TINFO, "[%d] value:%lld time_enabled:%lld "
		       "time_running:%lld", i, diff.value,
		       diff.time_enabled, diff.time_running);

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
		if (diff.time_enabled != diff.time_running) {
			hwctrs = i;
			break;
		}
	}

	for (i = 0; i <= hwctrs; i++)
		SAFE_CLOSE(fdarry[i]);

	return hwctrs;
}
#endif /* __s390__ */

static void bind_to_current_cpu(void)
{
#ifdef HAVE_SCHED_GETCPU
	int cpu = sched_getcpu();
	size_t mask_size;
	cpu_set_t *mask;

	if (cpu == -1)
		tst_brk(TBROK | TERRNO, "sched_getcpu() failed");

	mask = CPU_ALLOC(cpu + 1);
	mask_size = CPU_ALLOC_SIZE(cpu + 1);
	CPU_ZERO_S(mask_size, mask);
	CPU_SET(cpu, mask);
	if (sched_setaffinity(0, mask_size, mask) == -1)
		tst_brk(TBROK | TERRNO, "sched_setaffinity() failed");
	CPU_FREE(mask);
#endif
}

static void setup(void)
{
	int i;
	struct perf_event_attr tsk_event, hw_event;

	for (i = 0; i < MAX_CTRS; i++) {
		hwfd[i] = -1;
		tskfd[i] = -1;
	}

	bench_work(500);

	/*
	 * According to perf_event_open's manpage, the official way of
	 * knowing if perf_event_open() support is enabled is checking for
	 * the existence of the file /proc/sys/kernel/perf_event_paranoid.
	 */
	if (access("/proc/sys/kernel/perf_event_paranoid", F_OK) == -1)
		tst_brk(TCONF, "Kernel doesn't have perf_event support");

	bind_to_current_cpu();
#ifdef __s390__
	/*
	 * On s390 the "time_enabled" and "time_running" values are always the
	 * same, therefore count_hardware_counters() does not work.
	 *
	 * There are distinct/dedicated counters that can be used independently.
	 * Use the dedicated counter for instructions here.
	 */
	ntotal = nhw = 1;
#else
	nhw = count_hardware_counters();
	ntotal = nhw + 4;
#endif

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
	tsk_event.disabled = 0;
	for (i = 0; i < ntotal; ++i) {
		hwfd[i] = perf_event_open(&hw_event, 0, -1, -1, 0);
		tskfd[i] = perf_event_open(&tsk_event, 0, -1, hwfd[i], 0);
	}
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < ntotal; i++) {
		if (hwfd[i] != -1)
			SAFE_CLOSE(hwfd[i]);
		if (tskfd[i] != -1)
			SAFE_CLOSE(tskfd[i]);
	}

	if (tsk0 != -1)
		SAFE_CLOSE(tsk0);
}

static void verify(void)
{
	unsigned long long vt0, vt[MAX_CTRS], vh[MAX_CTRS];
	unsigned long long vtsum = 0, vhsum = 0;
	int i;
	double ratio;
	struct sched_param sparam = {.sched_priority = 1};

	if (sched_setscheduler(0, SCHED_FIFO, &sparam)) {
		tst_brk(TBROK | TERRNO,
			"sched_setscheduler(0, SCHED_FIFO, ...) failed");
	}

	all_counters_set(PR_TASK_PERF_EVENTS_ENABLE);
	do_work(8);
	/* stop groups with hw counters first before tsk0 */
	for (i = 0; i < ntotal; i++) {
		ioctl(hwfd[i], PERF_EVENT_IOC_DISABLE);
		ioctl(tskfd[i], PERF_EVENT_IOC_DISABLE);
	}
	all_counters_set(PR_TASK_PERF_EVENTS_DISABLE);

	sparam.sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &sparam)) {
		tst_brk(TBROK | TERRNO,
			"sched_setscheduler(0, SCHED_OTHER, ...) failed");
	}

	if (read(tsk0, &vt0, sizeof(vt0)) != sizeof(vt0))
		tst_brk(TBROK | TERRNO, "error reading task clock counter");

	for (i = 0; i < ntotal; ++i) {
		if (read(tskfd[i], &vt[i], sizeof(vt[i])) != sizeof(vt[i]) ||
		    read(hwfd[i], &vh[i], sizeof(vh[i])) != sizeof(vh[i]))
			tst_brk(TBROK | TERRNO, "error reading counter(s)");
		vtsum += vt[i];
		vhsum += vh[i];
	}

	tst_res(TINFO, "nhw: %d, overall task clock: %llu", nhw, vt0);
	tst_res(TINFO, "hw sum: %llu, task clock sum: %llu", vhsum, vtsum);

	if (verbose) {
		tst_res(TINFO, "hw counters:");
		for (i = 0; i < ntotal; ++i)
			tst_res(TINFO, " %llu", vh[i]);
		tst_res(TINFO, "task clock counters:");
		for (i = 0; i < ntotal; ++i)
			tst_res(TINFO, " %llu", vt[i]);
	}

	ratio = (double)vtsum / vt0;
	tst_res(TINFO, "ratio: %lf", ratio);
	if (ratio > nhw + 0.0001) {
		tst_res(TFAIL, "test failed (ratio was greater than %d)", nhw);
	} else {
		tst_res(TPASS, "test passed");
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.options = (struct tst_option[]) {
		{"v", &verbose, "Verbose output"},
		{},
	},
	.test_all = verify,
	.needs_root = 1,
	.max_runtime = 72
};
