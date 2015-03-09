/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007				      */
/*									      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.					      */
/*									      */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		      */
/* the GNU General Public License for more details.			      */
/*									      */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software		      */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*									      */
/******************************************************************************/
/******************************************************************************/
/*									      */
/* File:        sched_getaffinity01.c					      */
/*									      */
/* Description: This tests the sched_getaffinity() syscall		      */
/*									      */
/* Usage:  <for command-line>						      */
/* sched_getaffinity01 [-c n] [-e][-i n] [-I x] [-p x] [-t]		      */
/*      where,  -c n : Run n copies concurrently.			      */
/*			-e   : Turn on errno logging.			      */
/*			-i n : Execute test n times.			      */
/*			-I x : Execute test for x seconds.		      */
/*			-P x : Pause for x seconds between iterations.	      */
/*			-t   : Turn on syscall timing.			      */
/*									      */
/* Total Tests: 1							      */
/*									      */
/* Test Name:   sched_getaffinity01					      */
/* History:     Porting from Crackerjack to LTP is done by		      */
/*			Manas Kumar Nayak maknayak@in.ibm.com>		      */
/******************************************************************************/
#define _GNU_SOURCE
#define __USE_GNU
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "linux_syscall_numbers.h"

char *TCID = "sched_getaffinity01";
int TST_TOTAL = 1;

static long num;
static void do_test(void);
static void setup(void);
static void cleanup(void);

#define QUICK_TEST(t) \
do { \
	TEST(t); \
	tst_resm((TEST_RETURN == -1 ? TPASS : TFAIL) | TTERRNO, #t); \
} while (0)

#if !(__GLIBC_PREREQ(2, 7))
#define CPU_FREE(ptr)	free(ptr)
#endif

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);
	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		do_test();
	}

	cleanup();
	tst_exit();
}

static void do_test(void)
{
	int i;
	cpu_set_t *mask;
	int nrcpus = 1024;
	pid_t unused_pid;
	unsigned len;

#if __GLIBC_PREREQ(2, 7)
realloc:
	mask = CPU_ALLOC(nrcpus);
#else
	mask = malloc(sizeof(cpu_set_t));
#endif
	if (mask == NULL)
		tst_brkm(TFAIL | TTERRNO, cleanup, "fail to get enough memory");
#if __GLIBC_PREREQ(2, 7)
	len = CPU_ALLOC_SIZE(nrcpus);
	CPU_ZERO_S(len, mask);
#else
	len = sizeof(cpu_set_t);
	CPU_ZERO(mask);
#endif
	/* positive test */
	TEST(sched_getaffinity(0, len, mask));
	if (TEST_RETURN == -1) {
		CPU_FREE(mask);
#if __GLIBC_PREREQ(2, 7)
		if (errno == EINVAL && nrcpus < (1024 << 8)) {
			nrcpus = nrcpus << 2;
			goto realloc;
		}
#else
		if (errno == EINVAL)
			tst_resm(TFAIL, "NR_CPUS > 1024, we'd better use a "
				 "newer glibc(>= 2.7)");
		else
#endif
			tst_resm(TFAIL | TTERRNO, "fail to get cpu affinity");
		cleanup();
	} else {
		tst_resm(TINFO, "cpusetsize is %d", len);
		tst_resm(TINFO, "mask.__bits[0] = %lu ", mask->__bits[0]);
		for (i = 0; i < num; i++) {
#if __GLIBC_PREREQ(2, 7)
			TEST(CPU_ISSET_S(i, len, mask));
#else
			TEST(CPU_ISSET(i, mask));
#endif
			if (TEST_RETURN != -1)
				tst_resm(TPASS, "sched_getaffinity() succeed, "
					 "this process %d is running "
					 "processor: %d", getpid(), i);
		}
	}

#if __GLIBC_PREREQ(2, 7)
	CPU_ZERO_S(len, mask);
#else
	CPU_ZERO(mask);
#endif
	/* negative tests */
	QUICK_TEST(sched_getaffinity(0, len, (cpu_set_t *) - 1));
	QUICK_TEST(sched_getaffinity(0, 0, mask));

	unused_pid = tst_get_unused_pid(cleanup);
	QUICK_TEST(sched_getaffinity(unused_pid, len, mask));
	CPU_FREE(mask);
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();

	num = sysconf(_SC_NPROCESSORS_CONF);
	if (num == -1)
		tst_brkm(TBROK | TERRNO, NULL, "sysconf");
	tst_resm(TINFO, "system has %ld processor(s).", num);
}

static void cleanup(void)
{
	tst_rmdir();
}
