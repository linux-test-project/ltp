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

#ifndef RUSAGE_THREAD
#define RUSAGE_THREAD 1
#endif

static void busyloop(long wait);
static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	struct rusage usage;
	unsigned long ulast, udelta, slast, sdelta;
	int i, lc;
	char *msg;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

#if (__powerpc__) || (__powerpc64__) || (__s390__) || (__s390x__)
	tst_brkm(TCONF, NULL, "This test is not designed for current system");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0; i = 0;
		ulast = 0, slast = 0;
		SAFE_GETRUSAGE(cleanup, RUSAGE_THREAD, &usage);
		tst_resm(TINFO, "utime:%12luus; stime:%12luus",
			usage.ru_utime.tv_usec, usage.ru_stime.tv_usec);
		while (i < RECORD_MAX) {
			SAFE_GETRUSAGE(cleanup, RUSAGE_THREAD, &usage);
			udelta = usage.ru_utime.tv_usec - ulast;
			sdelta = usage.ru_stime.tv_usec - slast;
			if (udelta > 0 || sdelta > 0) {
				i++;
				tst_resm(TINFO, "utime:%12luus; stime:%12luus",
					    usage.ru_utime.tv_usec,
					    usage.ru_stime.tv_usec);
				if (udelta > 1000+BIAS_MAX)
					tst_brkm(TFAIL, cleanup,
						    "utime increased > 1000us:"
						    " delta = %luus", udelta);
				if (sdelta > 1000+BIAS_MAX)
					tst_brkm(TFAIL, cleanup,
						    "stime increased > 1000us:"
						    " delta = %luus", sdelta);
			}
			ulast = usage.ru_utime.tv_usec;
			slast = usage.ru_stime.tv_usec;
			busyloop(100000);
		}
	}
	cleanup();
	tst_exit();
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
