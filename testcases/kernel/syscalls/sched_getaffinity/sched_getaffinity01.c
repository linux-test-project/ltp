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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
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

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "sched_getaffinity01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 1;		     /* total number of tests in this file.   */

/* Extern Global Functions */
/******************************************************************************/
/*									      */
/* Function:    cleanup							      */
/*									      */
/* Description: Performs all one time clean up for this test on successful    */
/*		completion,  premature exit or  failure. Closes all temporary */
/*		files, removes all temporary directories exits the test with  */
/*		appropriate return code by calling tst_exit() function.       */
/*									      */
/* Input:       None.							      */
/*									      */
/* Output:      None.							      */
/*									      */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*		On success - Exits calling tst_exit(). With '0' return code.  */
/*									      */
/******************************************************************************/
extern void cleanup() {

	TEST_CLEANUP;
	tst_rmdir();

}

/* Local  Functions */
/******************************************************************************/
/*									      */
/* Function:    setup							      */
/*									      */
/* Description: Performs all one time setup for this test. This function is   */
/*		typically used to capture signals, create temporary dirs      */
/*		and temporary files that may be used in the course of this    */
/*		test.						      	      */
/*									      */
/* Input:       None.							      */
/*									      */
/* Output:      None.							      */
/*									      */
/* Return:      On failure - Exits by calling cleanup().		      */
/*		On success - returns 0.					      */
/*									      */
/******************************************************************************/
void setup() {
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

#define QUICK_TEST(t) \
do { \
	TEST(t); \
	tst_resm((TEST_RETURN == -1 ? TPASS : TFAIL) | TTERRNO, #t); \
} while (0)

#if !(__GLIBC_PREREQ(2,7))
#define CPU_FREE(ptr)	free(ptr)
#endif
int main(int ac, char **av) {
	int lc, num, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */
	cpu_set_t *mask;
	int nrcpus = 1024;
	pid_t pid;
	unsigned int len;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST(num=sysconf(_SC_NPROCESSORS_CONF));  //the number of processor(s)
	tst_resm(TINFO,"system has %d processor(s).", num);

	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		Tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

#if __GLIBC_PREREQ(2,7)
realloc:
			mask = CPU_ALLOC(nrcpus);
# else
			mask = malloc(sizeof(cpu_set_t));
#endif
			if (mask == NULL) {
				tst_brkm(TFAIL|TTERRNO, cleanup, "cann't get enough memory");
			}

#if __GLIBC_PREREQ(2,7)
			len = CPU_ALLOC_SIZE(nrcpus);
			CPU_ZERO_S(len, mask); //clear
#else
			len = sizeof(cpu_set_t);
			CPU_ZERO(mask); //clear
#endif
			TEST(sched_getaffinity(0, len, mask));     //call sched_getaffinity()
			if (TEST_RETURN == -1) {

				CPU_FREE(mask);
#if __GLIBC_PREREQ(2,7)
				if (errno == EINVAL && nrcpus < (1024 << 8)) {
					nrcpus = nrcpus << 2;
					goto realloc;
				}
#else
				if (errno == EINVAL)
					tst_resm(TFAIL, "NR_CPUS of the kernel is more than 1024, so we'd better use a newer glibc(>= 2.7)");
				else
#endif
					tst_resm(TFAIL|TTERRNO, "could not get cpu affinity");
				cleanup();
			} else {
				tst_resm(TINFO,"cpusetsize is %d", len);
				tst_resm(TINFO,"mask.__bits[0] = %lu ",mask->__bits[0]);
				for (i=0;i<num;i++) {    // check the processor
#if __GLIBC_PREREQ(2,7)
					TEST(CPU_ISSET_S(i, len, mask));
#else
					TEST(CPU_ISSET(i, mask));
#endif
					if (TEST_RETURN != -1) {
						tst_resm(TPASS,"sched_getaffinity() succeed ,this process %d is running processor: %d",getpid(), i);
					}
				}
			}

#if __GLIBC_PREREQ(2,7)
			CPU_ZERO_S(len, mask); //clear
#else
			CPU_ZERO(mask);	//clear
#endif
			QUICK_TEST(sched_getaffinity(0, len, (cpu_set_t *)-1));
			QUICK_TEST(sched_getaffinity(0, 0, mask));
			QUICK_TEST(sched_getaffinity(getpid() + 1, len, mask));
			/*
			 * pid_t -> int -- the actual kernel limit is lower
			 * though, but this is a negative test, not a positive
			 * one.
			 *
			 * Push comes to shove, if the user doesn't have the
			 * ability to kill(3) processes (errno = EPERM), then
			 * set the pid to the highest possible represented
			 * value and cross your fingers in the hope that
			 * a) Linux somehow hasn't started allocating PIDs
			 * this high and b) the PID = INT_MAX isn't in fact
			 * running.
			 */
			for (pid = 2; pid < INT_MAX; pid++) {

				if (kill(pid, 0) == -1) {

					if (errno == ESRCH) {
						break;
					} else if (errno == EPERM) {
						pid = INT_MAX-1;
					}

				}

			}

			QUICK_TEST(sched_getaffinity(pid, len, mask));
			CPU_FREE(mask);

		}

	}

	cleanup();

	tst_exit();
}
