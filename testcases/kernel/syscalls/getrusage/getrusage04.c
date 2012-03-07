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

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

char *TCID = "getrusage04";
int TST_TOTAL = 1;

#define BIAS_MAX      1000
#define RECORD_MAX    20
#define FACTOR_MAX    10

#ifndef RUSAGE_THREAD
#define RUSAGE_THREAD 1
#endif

static int opt_factor;
static char *factor_str;
static long factor_nr = 1;

option_t child_options[] = {
	{ "m:", &opt_factor, &factor_str },
	{ NULL, NULL,         NULL }
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
	char *msg;
	char msg_string[BUFSIZ];

	msg = parse_opts(argc, argv, child_options, fusage);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

#if (__powerpc__) || (__powerpc64__) || (__s390__) || (__s390x__)
	tst_brkm(TCONF, NULL, "This test is not designed for current system");
#endif

	setup();

	if (opt_factor)
		factor_nr = SAFE_STRTOL(cleanup, factor_str, 0, FACTOR_MAX);

	tst_resm(TINFO, "Using %ld as multiply factor for max [us]time "
		 "increment (1000+%ldus)!", factor_nr, BIAS_MAX * factor_nr);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0; i = 0;
		SAFE_GETRUSAGE(cleanup, RUSAGE_THREAD, &usage);
		tst_resm(TINFO, "utime:%12luus; stime:%12luus",
			usage.ru_utime.tv_usec, usage.ru_stime.tv_usec);
		ulast = usage.ru_utime.tv_usec;
		slast = usage.ru_stime.tv_usec;

		while (i < RECORD_MAX) {
			SAFE_GETRUSAGE(cleanup, RUSAGE_THREAD, &usage);
			udelta = usage.ru_utime.tv_usec - ulast;
			sdelta = usage.ru_stime.tv_usec - slast;
			if (udelta > 0 || sdelta > 0) {
				i++;
				tst_resm(TINFO, "utime:%12luus; stime:%12luus",
					    usage.ru_utime.tv_usec,
					    usage.ru_stime.tv_usec);
				if (udelta > 1000+(BIAS_MAX * factor_nr)) {
					sprintf(msg_string,
						"utime increased > %ldus:",
						1000 + BIAS_MAX * factor_nr);
					tst_brkm(TFAIL, cleanup, msg_string,
						    " delta = %luus", udelta);
				}
				if (sdelta > 1000+(BIAS_MAX * factor_nr)) {
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
	while (wait--)
		;
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;
}
