/*
 * getrusage04 - accuracy of getrusage() with RUSAGE_THREAD
 *
 * This program is used for testing the following upstream commit:
 * 761b1d26df542fd5eb348837351e4d2f3bc7bffe.
 *
 * getrusage() returns cpu resource usage with accuracy of 10ms
 * when RUSAGE_THREAD is specified to the argument who. Meanwhile,
 * accuracy is 1ms when RUSAGE_SELF is specified. This bad accuracy
 * of getrusage() caused a big impact on some application which is
 * critical to accuracy of cpu usage. The upstream fix removed
 * casts to clock_t in task_u/stime(), to keep granularity of
 * cputime_t over the calculation.
 *
 * Note that the accuracy on powerpc and s390x systems is always
 * 10ms when either RUSAGE_THREAD or RUSAGE_SELF specified, so
 * this test won't be executed on those platforms.
 *
 * Copyright (C) 2011  Red Hat, Inc.
 * Copyright (C) 2013  Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/posix_clocks.h"

char *TCID = "getrusage04";
int TST_TOTAL = 1;

#define RECORD_MAX    20
#define FACTOR_MAX    10

#ifndef RUSAGE_THREAD
#define RUSAGE_THREAD 1
#endif

static long BIAS_MAX;

static int opt_factor;
static char *factor_str;
static long factor_nr = 1;

option_t child_options[] = {
	{"m:", &opt_factor, &factor_str},
	{NULL, NULL, NULL}
};

static void fusage(void);
static void busyloop(long wait);
static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	struct rusage usage;
	unsigned long ulast, udelta, slast, sdelta;
	int i, lc;
	char msg_string[BUFSIZ];

	tst_parse_opts(argc, argv, child_options, fusage);

#if (__powerpc__) || (__powerpc64__) || (__s390__) || (__s390x__)
	tst_brkm(TCONF, NULL, "This test is not designed for current system");
#endif

	setup();

	if (opt_factor)
		factor_nr = SAFE_STRTOL(cleanup, factor_str, 0, FACTOR_MAX);

	tst_resm(TINFO, "Using %ld as multiply factor for max [us]time "
		 "increment (1000+%ldus)!", factor_nr, BIAS_MAX * factor_nr);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		i = 0;
		SAFE_GETRUSAGE(cleanup, RUSAGE_THREAD, &usage);
		tst_resm(TINFO, "utime:%12lldus; stime:%12lldus",
			 (long long)usage.ru_utime.tv_usec,
			 (long long)usage.ru_stime.tv_usec);
		ulast = usage.ru_utime.tv_usec;
		slast = usage.ru_stime.tv_usec;

		while (i < RECORD_MAX) {
			SAFE_GETRUSAGE(cleanup, RUSAGE_THREAD, &usage);
			udelta = usage.ru_utime.tv_usec - ulast;
			sdelta = usage.ru_stime.tv_usec - slast;
			if (udelta > 0 || sdelta > 0) {
				i++;
				tst_resm(TINFO, "utime:%12lldus; stime:%12lldus",
					 (long long)usage.ru_utime.tv_usec,
					 (long long)usage.ru_stime.tv_usec);
				if ((long)udelta > 1000 + (BIAS_MAX * factor_nr)) {
					sprintf(msg_string,
						"utime increased > %ldus:",
						1000 + BIAS_MAX * factor_nr);
					tst_brkm(TFAIL, cleanup, msg_string,
						 " delta = %luus", udelta);
				}
				if ((long)sdelta > 1000 + (BIAS_MAX * factor_nr)) {
					sprintf(msg_string,
						"stime increased > %ldus:",
						1000 + BIAS_MAX * factor_nr);
					tst_brkm(TFAIL, cleanup, msg_string,
						 " delta = %luus", sdelta);
				}
			}
			ulast = usage.ru_utime.tv_usec;
			slast = usage.ru_stime.tv_usec;
			busyloop(100000);
		}
	}

	tst_resm(TPASS, "Test Passed");

	cleanup();
	tst_exit();
}

static void fusage(void)
{
	printf("  -m n    use n as multiply factor for max [us]time "
	       "increment (1000+(1000*n)us),\n          default value is 1\n");
}

static void busyloop(long wait)
{
	while (wait--) ;
}

/*
 * The resolution of getrusage timers currently depends on CONFIG_HZ settings,
 * as they are measured in jiffies.
 *
 * The problem is that there is no reasonable API to get either getrusage
 * timers resolution or duration of jiffie.
 *
 * Here we use clock_getres() with linux specific CLOCK_REALTIME_COARSE (added
 * in 2.6.32) which is also based on jiffies. This timer has the same
 * granularity as getrusage but it's not guaranteed and it may change in the
 * future.
 *
 * The default value for resolution was choosen to be 4ms as it corresponds to
 * CONFIG_HZ=250 which seems to be default value.
 */
static unsigned long guess_timer_resolution(void)
{
	struct timespec res;

	if (clock_getres(CLOCK_REALTIME_COARSE, &res)) {
		tst_resm(TINFO,
		        "CLOCK_REALTIME_COARSE not supported, using 4000 us");
		return 4000;
	}

	if (res.tv_nsec < 1000000 || res.tv_nsec > 10000000) {
		tst_resm(TINFO, "Unexpected CLOCK_REALTIME_COARSE resolution,"
		        " using 4000 us");
		return 4000;
	}

	tst_resm(TINFO, "Expected timers granularity is %li us",
	         res.tv_nsec / 1000);

	return res.tv_nsec / 1000;
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	if (tst_is_virt(VIRT_XEN) || tst_is_virt(VIRT_KVM) || tst_is_virt(VIRT_HYPERV))
		tst_brkm(TCONF, NULL, "This testcase is not supported on this"
		        " virtual machine.");

	BIAS_MAX = guess_timer_resolution();

	TEST_PAUSE;
}

static void cleanup(void)
{
}
