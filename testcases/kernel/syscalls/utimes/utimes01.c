/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007 ,Hitachi, Ltd		     */
/*	  Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,	      */
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
/* File:	utimes01.c						    */
/*									    */
/* Description: This tests the utimes() syscall			       */
/*									      */
/* 									      */
/*									      */
/*									      */
/*									      */
/*									    */
/* Usage:  <for command-line>						 */
/* utimes01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			      */
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   utimes01						      */
/* History:     Porting from Crackerjack to LTP is done by		    */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "../utils/include_j_h.h"
#include "../utils/common_j_h.c"

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Global Variables */
char *TCID = "utimes01";  /* Test program identifier.*/
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
	tst_require_root(NULL);

	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

/*
 * Macros
 */
#define SYSCALL_NAME    "utimes"

enum test_type {
	NORMAL,
	FILE_NOT_EXIST,
	NO_FNAME,
};

/*
 * Data Structure
 */
struct test_case {
	int ttype;
	long a_sec;
	long m_sec;
	char *user;
	int ret;
	int err;

};

/* Test cases
 *
 *   test status of errors on man page
 *
 *   EACCES	     v (permission denied)
 *   ENOENT	     v (file does not exist)
 *
 *   test status of errors on man page
 *
 *   EFAULT	     v (points to not process address space)
 */

static struct test_case tcase[] = {
	{ // case00
		.ttype	  = NORMAL,
		.a_sec	  = 0,
		.m_sec	  = 1000,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case01
		.ttype	  = NORMAL,
		.a_sec	  = 1000,
		.m_sec	  = 0,
		.ret	    = 0,
		.err	    = 0,
	},
	{ // case02
		.ttype	  = NORMAL,
		.user	   = "nobody",
		.ret	    = -1,
		.err	    = EACCES, // RHEL4U1 + 2.6.18 returns EPERM
	},
	{ // case03
		.ttype	  = FILE_NOT_EXIST,
		.a_sec	  = 1000,
		.m_sec	  = 2000,
		.ret	    = -1,
		.err	    = ENOENT,
	},

	{ // case04
		.ttype	  = NO_FNAME,
		.a_sec	  = 1000,
		.m_sec	  = 2000,
		.ret	    = -1,
		.err	    = EFAULT,
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
	struct timeval tv[2];
	char fpath[PATH_MAX], c = '\0';
	int rc, len, cmp_ok = 1;
	struct stat st;
	uid_t old_uid;

	/* XXX (garrcoop): memory leak with get_tst_tmpdir. */
	TEST(rc = setup_file(get_tst_tmpdir(), "test.file", fpath));
	if (rc < 0)
		return 1;
	/* The test just needs the file, so no need to keep it open. */
	close(rc);

	/*
	 * Change effective user id
	 */
	if (tc->user != NULL) {
		TEST(rc = setup_euid(tc->user, &old_uid));
		if (rc < 0)
			goto EXIT2;
	}

	/*
	 * Execute system call
	 */
	memset(tv, 0, 2 * sizeof(struct timeval));
	tv[0].tv_sec = tc->a_sec;
	tv[1].tv_sec = tc->m_sec;
	TEST(len = strlen(fpath));
	if (tc->ttype == FILE_NOT_EXIST) {
		c = fpath[len - 1];
		fpath[len - 1] = '\0';
	}
	errno = 0;
	if (tc->ttype == NO_FNAME) {
		/**
		 * Note (garrcoop):
		 *
		 * If you do NULL directly, then gcc [4.3] will complain when
		 * one specifies -Wnonnull in CPPFLAGS. This is a negative
		 * test, but let's not allow the compiler to complain about
		 * something trivial like this.
		 **/
		const char *dummy = NULL;
		TEST(sys_ret = utimes(dummy, tv));
	}
	else {
		if (tc->user == NULL)
			TEST(sys_ret = utimes(fpath, tv));
		else
			TEST(sys_ret = utimes(fpath, NULL));
	}
        tv[0].tv_sec = tc->a_sec;
        tv[1].tv_sec = tc->m_sec;
        TEST(len = strlen(fpath));
        if (tc->ttype == FILE_NOT_EXIST) {
                c = fpath[len - 1];
                fpath[len - 1] = '\0';
        }
        errno = 0;
        if (tc->ttype == NO_FNAME) {
                /**
                 * Note (garrcoop):
                 *
                 * If you do NULL directly, then gcc [4.3] will complain when
                 * one specifies -Wnonnull in CPPFLAGS. This is a negative
                 * test, but let's not allow the compiler to complain about
                 * something trivial like this.
                 **/
                const char *dummy = NULL;
                TEST(sys_ret = utimes(dummy, tv));
        }
        else {
                if (tc->user == NULL)
                        TEST(sys_ret = utimes(fpath, tv));
                else
                        TEST(sys_ret = utimes(fpath, NULL));
        }
	sys_errno = errno;
	if (tc->ttype == FILE_NOT_EXIST)
		fpath[len - 1] = c;
	if (sys_ret < 0)
		goto TEST_END;

	/*
	 * Check test file's time stamp
	 */
	rc = stat(fpath, &st);
	if (rc < 0) {
		EPRINTF("stat failed.\n");
		result = 1;
		goto EXIT1;
	}
	tst_resm(TINFO,"E:%ld,%ld <=> R:%ld,%ld",tv[0].tv_sec, tv[1].tv_sec, st.st_atime, st.st_mtime);
	cmp_ok = st.st_atime == tv[0].tv_sec && st.st_mtime == tv[1].tv_sec;

	/*
	 * Check results
	 */
TEST_END:
	result |= (sys_errno != tc->err) || !cmp_ok;
	PRINT_RESULT_CMP(sys_ret >= 0, tc->ret, tc->err, sys_ret, sys_errno,cmp_ok);

	/*
	 * Restore effective user id
	 */
EXIT1:
	if (tc->user != NULL) {
		TEST(rc = cleanup_euid(old_uid));
		if (rc < 0)
			return 1;
	}
EXIT2:
	TEST(cleanup_file(fpath));

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

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
	     tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			for (i = 0; i < (int)(sizeof(tcase) / sizeof(tcase[0])); i++) {
				int ret;
				tst_resm(TINFO, "(case%02d) START", i);
				ret = do_test(&tcase[i]);
				tst_resm(TINFO, "(case%02d) END => %s",
					i, (ret == 0) ? "OK" : "NG");
				result |= ret;
			}

        	}
	}
	cleanup();
	tst_exit();
}