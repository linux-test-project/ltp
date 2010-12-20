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
/* File:	mq_open01.c						   */
/*									    */
/* Description: This tests the mq_open() syscall			      */
/*									      */
/* 									      */
/*									      */
/*									      */
/*									      */
/*									    */
/* Usage:  <for command-line>						 */
/* mq_open01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			     */
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   mq_open01						     */
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
char *TCID = "mq_open01";  /* Test program identifier.*/
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
void cleanup(void) {

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
#define SYSCALL_NAME    "mq_open"

enum test_type {
	NORMAL,
	NO_FILE,
	NO_SPACE,
};

/*
 * Data Structure
 */
struct test_case {
	int ttype;
	char *user;
	char *qname;
	int oflag;
	long mq_maxmsg;	 // Maximum numebr of messages.
	long mq_msgsize;	// Maximum message size.
	int ret;
	int err;
};

#define ULIMIT_FNUM     0
#define PROC_MAX_QUEUES "/proc/sys/fs/mqueue/queues_max"

/* Test cases
 *
 *   test status of errors on man page
 *
 *   EACCES	     v (permission is denied)
 *   EEXIST	     v (named message queue already exists)
 *   EINTR	      --- (interrupted by a signal)
 *   EINVAL	     v (not supported for the given name, or invalid
 *			 arguments)
 *   EMFILE	     v (process file table overflow)
 *   ENAMETOOLONG       v (too long name length)
 *   ENFILE	     can't check because it's difficult to create no-fd
 *   ENOENT	     v (O_CREAT is not set and the named message queue
 *			 does not exist)
 *   ENOSPC	     v (no space for the new message queue)
 */

static struct test_case tcase[] = {
#if 1
	{ // case00
		.ttype	  = NORMAL,
		.qname	  = QUEUE_NAME,
		.oflag	  = O_CREAT,
		.mq_maxmsg      = 20,
		.mq_msgsize     = 16384,
		.ret	    = 0,
		.err	    = 0,
	},
#else
	{ // case00
		.ttype	  = NORMAL,
		.qname	  = QUEUE_NAME,
		.oflag	  = O_CREAT,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case01
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
		.oflag	  = O_CREAT,
		.ret	    = 0,
		.err	    = 0,
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
				   "aaaaaaaaaaaaaaaa",
		.oflag	  = O_CREAT,
		.ret	    = -1,
		.err	    = ENAMETOOLONG,
	},

	{ // case03
		.ttype	  = NORMAL,
		.qname	  = "",
		.oflag	  = O_CREAT,
		.ret	    = -1,
		.err	    = EINVAL,
	},
	{ // case04
		.ttype	  = NORMAL,
		.user	   = "nobody",
		.qname	  = QUEUE_NAME,
		.ret	    = -1,
		.err	    = EACCES,
	},
	{ // case05
		.ttype	  = NORMAL,
		.qname	  = QUEUE_NAME,
		.oflag	  = O_CREAT|O_EXCL,
		.ret	    = -1,
		.err	    = EEXIST,
	},
	{ // case06
		.ttype	  = NO_FILE,
		.qname	  = QUEUE_NAME,
		.oflag	  = O_CREAT,
		.ret	    = -1,
		.err	    = EMFILE,
	},
	{ // case07
		.ttype	  = NORMAL,
		.qname	  = "/notexist",
		.oflag	  = 0,
		.ret	    = -1,
		.err	    = ENOENT,
	},

	{ // case08
		.ttype	  = NO_SPACE,
		.user	   = "nobody",
		.qname	  = QUEUE_NAME,
		.oflag	  = O_CREAT,
		.ret	    = -1,
		.err	    = ENOSPC,
	},
#endif
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
	int rc, fd1 = -1, fd2 = -1, cmp_ok = 1;
	uid_t old_uid = -1;
	rlim_t oldlim = -1;
	int oldval = -1;
	struct mq_attr new, old, *p_attr;

	 /*
	 * When test ended with SIGTERM etc, mq discriptor is left remains.
	 * So we delete it first.
	 */
	TEST(mq_unlink(QUEUE_NAME));

	/*
	 * Execute system call
	 */

	if (tc->ttype != NO_SPACE && !(tc->oflag & O_CREAT)) {
		errno = 0;
		TEST(sys_ret = mq_open(QUEUE_NAME, O_CREAT|O_EXCL|O_RDWR, S_IRWXU, NULL));
		sys_errno = errno;
		if (sys_ret < 0)
			goto TEST_END;
		fd1 = sys_ret;
	}
	if (tc->ttype == NO_FILE) {
		TEST(rc = setup_ulimit_fnum(ULIMIT_FNUM, &oldlim));
		if (rc < 0) {
			result = 1;
			goto EXIT;
		}
	}

	/*
	 * Change /proc/sys/fs/mqueue/queues_max
	 */
	if (tc->ttype == NO_SPACE) {
		TEST(rc = setup_proc_fs(PROC_MAX_QUEUES, 0, &oldval));
		if (rc < 0) {
			result = 1;
			goto EXIT;
		}
	}

	/*
	 * Change effective user id
	 */
	if (tc->user != NULL) {
		TEST(rc = setup_euid(tc->user, &old_uid));
		if (rc < 0) {
			result = 1;
			goto EXIT;
		}
	}

	 /*
	 * Execute system call
	 */
	//tst_resm(TINFO,"PATH_MAX: %d\n", PATH_MAX);
	//tst_resm(TINFO,"NAME_MAX: %d", NAME_MAX);
	p_attr = NULL;
	if (tc->mq_maxmsg || tc->mq_msgsize) {
		new.mq_maxmsg = tc->mq_maxmsg;
		new.mq_msgsize = tc->mq_msgsize;
		p_attr = &new;
	}
	errno = 0;
	if (tc->oflag & O_CREAT)
		TEST(sys_ret = mq_open(tc->qname, tc->oflag, S_IRWXU, p_attr));
	else
		TEST(sys_ret = mq_open(tc->qname, tc->oflag));
	sys_errno = errno;
	if (sys_ret < 0)
		goto TEST_END;
	fd2 = sys_ret;

	if (p_attr) {
		TEST(rc = syscall(__NR_mq_getsetattr, fd2, NULL, &old));
		if (TEST_RETURN < 0) {
		 	tst_resm(TFAIL, "mq_getsetattr failed - errno = %d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
			result = 1;
			goto EXIT;
		}
		tst_resm(TINFO,"mq_maxmsg E:%ld,\tR:%ld",new.mq_maxmsg, old.mq_maxmsg);
		tst_resm(TINFO,"mq_msgsize E:%ld,\tR:%ld",new.mq_msgsize, old.mq_msgsize);
		cmp_ok = old.mq_maxmsg == new.mq_maxmsg &&
			 old.mq_msgsize == new.mq_msgsize;
	}

TEST_END:
	/*
	 * Check results
	 */
	result |= (sys_errno != tc->err) || !cmp_ok;
	PRINT_RESULT_CMP(sys_ret >= 0, tc->ret, tc->err, sys_ret, sys_errno,cmp_ok);

EXIT:
	if (tc->user != NULL && old_uid != -1)
		TEST(cleanup_euid(old_uid));

	if (tc->ttype == NO_SPACE && oldval != -1)
		TEST(cleanup_proc_fs(PROC_MAX_QUEUES, oldval));

	if (tc->ttype == NO_FILE && oldlim != -1)
		TEST(cleanup_ulimit_fnum(oldlim));
	if (fd1 >= 0)
		TEST(close(fd1));
	if (fd2 >= 0)
		TEST(close(fd2));
	if (fd1 >= 0)
		TEST(mq_unlink(QUEUE_NAME));
	if (fd2 >= 0 && strcmp(tc->qname, QUEUE_NAME) != 0)
		TEST(mq_unlink(tc->qname));

	return result;
}

/*
 * main()
 */

int main(int ac, char **av) {
	int result = RESULT_OK;
	int i;
	int lc;		 /* loop counter */
	char *msg;	      /* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			/*
			 * Execute test
	 		 */
			for (i = 0; i < (int)(sizeof(tcase) / sizeof(tcase[0])); i++) {
				int ret;
				tst_resm(TINFO, "(case%02d) START", i);
				ret = do_test(&tcase[i]);
				tst_resm(TINFO, "(case%02d) END => %s", i,
					(ret == 0) ? "OK" : "NG");
				result |= ret;
			}

			/*
			 * Check results
	 		 */
			switch(result) {
			case RESULT_OK:
				tst_resm(TPASS, "mq_open call succeeded ");
				break;

			default:
			   	tst_brkm(TFAIL|TTERRNO, cleanup,
					"mq_open failed");
				break;
			}

		}
	}
	cleanup();
	tst_exit();
}