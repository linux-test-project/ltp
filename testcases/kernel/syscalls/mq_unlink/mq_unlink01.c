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
/* File:	mq_ulink01.c						  */
/*									    */
/* Description: This tests the mq_ulink() syscall			     */
/*									      */
/* 									      */
/*									      */
/*									      */
/*									      */
/*									    */
/* Usage:  <for command-line>						 */
/* mq_ulink01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			    */
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   mq_ulink01					     */
/* History:     Porting from Crackerjack to LTP is done by		    */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/******************************************************************************/

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <limits.h>

#include "../utils/include_j_h.h"
#include "../utils/common_j_h.c"

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "mq_ulink01";  /* Test program identifier.*/
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
	tst_require_root(tst_exit);
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

/*
 * Macros
 */
#define SYSCALL_NAME    "mq_ulink"

enum test_type {
	NORMAL,
};

/*
 * Data Structure
 */
struct test_case {
	char *user;
	char *qname;
	int ttype;
	int ret;
	int err;
};

/* Test cases
*
*   test status of errors on man page
*
*   EACCES	     v (permission is denied)
*   ENAMETOOLONG       v (too long name length)
*   ENOENT	     v (named message queue does not exist)
*/

static struct test_case tcase[] = {
	{ // case00
		.ttype	  = NORMAL,
		.qname	  = QUEUE_NAME,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case01
		.ttype	  = NORMAL,
		.user	   = "nobody",
		.qname	  = QUEUE_NAME,
		.ret	    = -1,
		.err	    = EACCES,
	},
	{ // case02
		.ttype	  = NORMAL,
				//  0	 1	 2	 3
				//  0123456789012345678901234567890123456789
		.qname	  = "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaa",
		.ret	    = -1,
		.err	    = ENOENT,
	},
	{ // case03
		.ttype	  = NORMAL,
				//  0	 1	 2	 3
				//  0123456789012345678901234567890123456789
		.qname	  = "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
				   "aaaaaaaaaaaaaaaa",
		.ret	    = -1,
		.err	    = ENAMETOOLONG,
	},
};

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
	int rc, fd1 = -1, fd2 = -1;
	uid_t old_uid = -1;

	/*
	 * When test ended with SIGTERM etc, mq discriptor is left remains.
	 * So we delete it first.
	 */
	TEST(mq_unlink(QUEUE_NAME));

	/*
	 * Open message queue
	 */
	rc = mq_open(QUEUE_NAME, O_CREAT|O_EXCL|O_RDWR, S_IRWXU, NULL);
	if (rc == -1) {
		tst_resm(TFAIL|TTERRNO, "mq_open failed");
		result = 1;
		goto EXIT;
	}
	fd1 = rc;

	/*
	 * Change effective user id
	 */
	if (tc->user != NULL) {
		TEST(rc = setup_euid(tc->user, &old_uid));
		if (TEST_RETURN < 0) {
			result = 1;
			goto EXIT;
		}
	}

	/*
	 * Execute system call
	 */
	errno = 0;
	TEST(sys_ret = mq_unlink(tc->qname));
	sys_errno = errno;
	if (sys_ret >= 0)
		fd2 = sys_ret;

	/*
	 * Check results
	 */
	result |= (sys_errno != tc->err);
	PRINT_RESULT(sys_ret >= 0, tc->ret, tc->err, sys_ret, sys_errno);

EXIT:
	if (tc->user != NULL && old_uid != -1)
		cleanup_euid(old_uid);

	if (fd1 >= 0)
		close(fd1);
	if (fd2 >= 0)
		close(fd2);
	mq_unlink(QUEUE_NAME);
	return 0;
}

/*
 * main()
 */

int main(int ac, char **av) {
	int i;
	int lc;
	char *msg;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			int ret;

			ret = 0;

			/*
			 * Execute test
			 */
			for (i = 0; ret == 0 &&
				    i < (int)(sizeof(tcase) / sizeof(tcase[0])); i++) {
				ret = do_test(&tcase[i]);
			}

		}
	}
	cleanup();
	tst_exit();
}
