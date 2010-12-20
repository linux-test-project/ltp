/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd		*/
/*	  Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,	      */
/*		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>, 	      */
/*		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>	      */
/*								  	      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or	  */
/* (at your option) any later version.					*/
/*									    */
/* This program is distributed in the hope that it will be useful,	    */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		  */
/* the GNU General Public License for more details.			   */
/*									    */
/* You should have received a copy of the GNU General Public License	  */
/* along with this program;  if not, write to the Free Software	       */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									    */
/******************************************************************************/
/******************************************************************************/
/*									    */
/* File:	clock_getres01.c					      */
/*									    */
/* Description: This tests the clock_getres() syscall			 */
/*									      */
/* 									      */
/*									      */
/*									      */
/*									      */
/*									    */
/* Usage:  <for command-line>						 */
/* clock_getres01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			*/
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   clock_getres01						*/
/* History:     Porting from Crackerjack to LTP is done by		    */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/******************************************************************************/
#include <sys/syscall.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "config.h"
#include "include_j_h.h"

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "clock_getres01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 1;		   /* total number of tests in this file.   */

/* Extern Global Functions */
/******************************************************************************/
/*									    */
/* Function:    cleanup						       */
/*									    */
/* Description: Performs all one time clean up for this test on successful    */
/*	      completion,  premature exit or  failure. Closes all temporary */
/*	      files, removes all temporary directories exits the test with  */
/*	      appropriate return code by calling tst_exit() function.       */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*	      On success - Exits calling tst_exit(). With '0' return code.  */
/*									    */
/******************************************************************************/
extern void cleanup() {

	TEST_CLEANUP;
	tst_rmdir();

}

/* Local  Functions */
/******************************************************************************/
/*									    */
/* Function:    setup							 */
/*									    */
/* Description: Performs all one time setup for this test. This function is   */
/*	      typically used to capture signals, create temporary dirs      */
/*	      and temporary files that may be used in the course of this    */
/*	      test.							 */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits by calling cleanup().		      */
/*	      On success - returns 0.				       */
/*									    */
/******************************************************************************/
void setup() {
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

/*
 * Macros
 */
#define SYSCALL_NAME    "clock_getres"

/*
 * Global variables
 */
static int opt_debug;
static char *progname;

enum test_type {
		NORMAL,
		NULL_POINTER,
};

/*
 * Data Structure
 */
struct test_case {
	clockid_t clk_id;
	int ttype;
	int ret;
	int err;
};

/* Test cases
*
*   test status of errors on man page
*
*   EPERM   can't check because clock_getres not requires permission
*   EINVAL  v (not supported clk_id)
*/

static struct test_case tcase[] = {
	{ // case00
		.clk_id	 = CLOCK_REALTIME,
		.ttype	  = NORMAL,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case01
		.clk_id	 = CLOCK_MONOTONIC,
		.ttype	  = NORMAL,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case02
		.clk_id	 = CLOCK_PROCESS_CPUTIME_ID,
		.ttype	  = NORMAL,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case03
		.clk_id	 = CLOCK_THREAD_CPUTIME_ID,
		.ttype	  = NORMAL,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case04
		.clk_id	 = -1,
		.ttype	  = NORMAL,
		.ret	    = -1,
		.err	    = EINVAL,
	},
	{ // case05
		.clk_id	 = CLOCK_REALTIME,
		.ttype	  = NULL_POINTER,
		.ret	    = 0,
		.err	    = 0,
	},
};

#define MEM_LENGTH	      (4 * 1024 * 1024)
/*
 * do_test()
 *
 *   Input  : TestCase Data
 *   Return : RESULT_OK(0), RESULT_NG(1)
 *
 */

static int do_test(struct test_case *tc)
{
	int sys_ret;
	int sys_errno;
	int result = RESULT_OK;
	struct timespec res;

	/*
	 * Execute system call
	 */
	errno = 0;
	if (tc->ttype == NULL_POINTER)
		TEST(sys_ret = syscall(__NR_clock_getres, tc->clk_id, NULL));
	else
		TEST(sys_ret = syscall(__NR_clock_getres, tc->clk_id, &res));
	sys_errno = errno;

	/*
	 * Check results
	 */
	result |= (sys_errno != tc->err);
	PRINT_RESULT(sys_ret >= 0, tc->ret, tc->err, sys_ret, sys_errno);

	return result;
}

/*
 * usage()
 */

static void usage(const char *progname)
{
	tst_resm(TINFO,"usage: %s [options]", progname);
	tst_resm(TINFO,"This is a regression test program of %s system call.",SYSCALL_NAME);
	tst_resm(TINFO,"options:");
	tst_resm(TINFO,"    -d --debug	   Show debug messages");
	tst_resm(TINFO,"    -h --help	    Show this message");

	exit(1);
}

/*
 * main()
 */

int main(int ac, char **av) {
	int result = RESULT_OK;
	int c, i;
	int lc;		/* loop counter */
	int ret;
	char *msg;      /* message returned from parse_opts */

	struct option long_options[] = {
		{ "debug",	no_argument,	0,	'd' },
		{ "help",	no_argument,	0,	'h' },
		{ NULL,		0,		NULL,	0 }
	};

	progname = basename(av[0]);

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			TEST(c = getopt_long(ac, av, "dh", long_options,
					     NULL));
			while (TEST_RETURN != -1) {
				switch (c) {
				case 'd':
					opt_debug = 1;
					break;
				default:
					usage(progname);

				}
			}

			if (ac != optind) {
				tst_resm(TINFO,"Options are not match.");
				usage(progname);

			}

			/*
			 * Execute test
		 	 */
			for (i = 0; i < (int)(sizeof(tcase) / sizeof(tcase[0])); i++) {
				tst_resm(TINFO,"(case%02d) START", i);
				ret = do_test(&tcase[i]);
				tst_resm((ret == 0 ? TPASS : TFAIL ), "(case%02d) END", i);
				result |= ret;
			}

			/*
			 * Check results
		 	*/
			switch(result) {
			case RESULT_OK:
				tst_resm(TPASS, "clock_getres call succeeded");
				break;

			default:
		 	   	tst_resm(TFAIL, "%s failed", TCID);
				RPRINTF("NG\n");
				cleanup();
				tst_exit();
				break;
			}

		}
	}
	cleanup();
	tst_exit();
}