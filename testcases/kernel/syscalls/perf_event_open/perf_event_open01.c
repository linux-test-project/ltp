/******************************************************************************/
/*                                                                            */
/* Ingo Molnar <mingo@elte.hu>, 2009                                          */
/* Copyright (c) Linux Test Project, 2014-2022                                */
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
 * Very simple performance counter testcase.
 * Picked up from: http://lkml.org/lkml/2008/12/5/17
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <linux/unistd.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include "config.h"
#include <linux/perf_event.h>

#include "test.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"

char *TCID = "perf_event_open01";

static void setup(void);
static void cleanup(void);

static struct test_case_t {
	uint32_t type;
	const char *config_name;
	unsigned long long config;
} event_types[] = {
	{ PERF_TYPE_HARDWARE, "PERF_COUNT_HW_INSTRUCTIONS",
	  PERF_COUNT_HW_INSTRUCTIONS },
	{ PERF_TYPE_HARDWARE, "PERF_COUNT_HW_CACHE_REFERENCES",
	  PERF_COUNT_HW_CACHE_REFERENCES },
	{ PERF_TYPE_HARDWARE, "PERF_COUNT_HW_CACHE_MISSES",
	  PERF_COUNT_HW_CACHE_MISSES },
	{ PERF_TYPE_HARDWARE, "PERF_COUNT_HW_BRANCH_INSTRUCTIONS",
	  PERF_COUNT_HW_BRANCH_INSTRUCTIONS },
	{ PERF_TYPE_HARDWARE, "PERF_COUNT_HW_BRANCH_MISSES",
	  PERF_COUNT_HW_BRANCH_MISSES },
	{ PERF_TYPE_SOFTWARE, "PERF_COUNT_SW_CPU_CLOCK",
	  PERF_COUNT_SW_CPU_CLOCK },
	{ PERF_TYPE_SOFTWARE, "PERF_COUNT_SW_TASK_CLOCK",
	  PERF_COUNT_SW_TASK_CLOCK },
};

int TST_TOTAL = ARRAY_SIZE(event_types);

static void verify(struct test_case_t *tc);
static struct perf_event_attr pe;

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			verify(&event_types[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	/*
	 * According to perf_event_open's manpage, the official way of
	 * knowing if perf_event_open() support is enabled is checking for
	 * the existence of the file /proc/sys/kernel/perf_event_paranoid.
	 */
	if (access("/proc/sys/kernel/perf_event_paranoid", F_OK) == -1)
		tst_brkm(TCONF, NULL, "Kernel doesn't have perf_event support");

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	pe.size = sizeof(struct perf_event_attr);
	pe.disabled = 1;
	pe.exclude_kernel = 1;
	pe.exclude_hv = 1;
}


static int perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
		int cpu, int group_fd, unsigned long flags)
{
	int ret;

	ret = tst_syscall(__NR_perf_event_open, hw_event, pid, cpu,
			  group_fd, flags);
	return ret;
}

/* do_work() is copied form performance_counter02.c */
#define LOOPS	100000000

static void do_work(void)
{
	int i;

	for (i = 0; i < LOOPS; ++i)
		asm volatile ("" : : "g" (i));
}

static void verify(struct test_case_t *tc)
{
	unsigned long long count;
	int fd, ret;

	pe.type = tc->type;
	pe.config = tc->config;

	TEST(perf_event_open(&pe, 0, -1, -1, 0));
	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == ENOENT || TEST_ERRNO == EOPNOTSUPP ||
		    TEST_ERRNO == ENODEV) {
			tst_resm(TCONF | TTERRNO,
			         "perf_event_open for %s not supported",
			         tc->config_name);
		} else {
			tst_brkm(TFAIL | TTERRNO, cleanup,
				 "perf_event_open %s failed unexpectedly",
				 tc->config_name);
		}
		return;
	}

	fd = TEST_RETURN;

	if (ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			 "ioctl set PERF_EVENT_IOC_RESET failed");
	}

	if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			 "ioctl set PERF_EVENT_IOC_ENABLE failed");
	}

	do_work();

	if (ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			 "ioctl set PERF_EVENT_IOC_RESET failed");
	}

	ret = read(fd, &count, sizeof(count));
	if (ret == sizeof(count)) {
		tst_resm(TINFO, "read event counter succeeded, "
			 "value: %llu", count);
		tst_resm(TPASS, "test PERF_TYPE_HARDWARE: %s succeeded",
			 tc->config_name);
	} else {
		tst_resm(TFAIL | TERRNO, "read event counter failed");
	}

	SAFE_CLOSE(cleanup, fd);

}

static void cleanup(void)
{
}
