/********************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd			*/
/*	  Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,		*/
/*		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,		*/
/*		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>		*/
/*										*/
/* This program is free software;  you can redistribute it and/or modify	*/
/* it under the terms of the GNU General Public License as published by		*/
/* the Free Software Foundation; either version 2 of the License, or		*/
/* (at your option) any later version.						*/
/*										*/
/* This program is distributed in the hope that it will be useful,		*/
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of		*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See			*/
/* the GNU General Public License for more details.				*/
/*										*/
/* You should have received a copy of the GNU General Public License		*/
/* along with this program;  if not, write to the Free Software			*/
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA	*/
/* USA										*/
/********************************************************************************/
/************************************************************************/
/*									*/
/* File:	mq_timedsend01.c					*/
/*									*/
/* Description: This tests the mq_timedsend() syscall			*/
/*									*/
/* 									*/
/*									*/
/*									*/
/*									*/
/*									*/
/* Usage:  <for command-line>						*/
/* mq_timedsend01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			*/
/*      where,  -c n : Run n copies concurrently.			*/
/*	      -e   : Turn on errno logging.				*/
/*	      -i n : Execute test n times.				*/
/*	      -I x : Execute test for x seconds.			*/
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									*/
/* Total Tests: 1							*/
/*									*/
/* Test Name:   mq_timedsend01						*/
/* History:     Porting from Crackerjack to LTP is done by		*/
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/************************************************************************/
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <signal.h>
#include <limits.h>

#include "../utils/include_j_h.h"
#include "../utils/common_j_h.c"

#include "test.h"
#include "linux_syscall_numbers.h"

char *TCID = "mq_timedsend01";
int testno;
int TST_TOTAL = 1;
struct sigaction act;

/*
 * sighandler()
 */
void sighandler(int sig)
{
	if (sig == SIGINT)
		return;
	return;
}

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
void cleanup(void)
{

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
void setup(void)
{

	/* Capture signals if any */
	act.sa_handler = sighandler;
	sigfillset(&act.sa_mask);

	sigaction(SIGINT, &act, NULL);
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

/*
 * Macros
 */
#define SYSCALL_NAME    "mq_timedsend"

enum test_type {
	NORMAL,
	FD_NONE,
	FD_NOT_EXIST,
	FD_FILE,
	FULL_QUEUE,
	SEND_SIGINT,
};

/*
 * Data Structure
 */
struct test_case {
	int ttype;
	int non_block;
	int len;
	unsigned prio;
	time_t sec;
	long nsec;
	int ret;
	int err;
};

#define MAX_MSG	 10
#define MAX_MSGSIZE     8192

/* Test cases
*
*   test status of errors on man page
*
*   EAGAIN	     v (would block)
*   EBADF	      v (not a valid descriptor)
*   EINTR	      v (interrupted by a signal)
*   EINVAL	     v (1. invalid 'msg_prio' or
*			 2. would block but timeout exists)
*   EMSGSIZE	   v ('msg_len' exceeds the message size of the queue)
*   ETIMEDOUT	  v (not block and timeout occured)
*/

static struct test_case tcase[] = {
	{			// case00
	 .ttype = NORMAL,
	 .len = 0,		// also success when size equals zero
	 .ret = 0,
	 .err = 0,
	 },
	{			// case01
	 .ttype = NORMAL,
	 .len = 1,
	 .ret = 0,
	 .err = 0,
	 },
	{			// case02
	 .ttype = NORMAL,
	 .len = MAX_MSGSIZE,
	 .ret = 0,
	 .err = 0,
	 },
	{			// case03
	 .ttype = NORMAL,
	 .len = 1,
	 .prio = 32767,		// max priority
	 .ret = 0,
	 .err = 0,
	 },
	{			// case04
	 .ttype = NORMAL,
	 .len = MAX_MSGSIZE + 1,
	 .ret = -1,
	 .err = EMSGSIZE,
	 },
	{			// case05
	 .ttype = FD_NONE,
	 .len = 0,
	 .ret = -1,
	 .err = EBADF,
	 },
	{			// case06
	 .ttype = FD_NOT_EXIST,
	 .len = 0,
	 .ret = -1,
	 .err = EBADF,
	 },
	{			// case07
	 .ttype = FD_FILE,
	 .len = 0,
	 .ret = -1,
	 .err = EBADF,
	 },
	{			// case08
	 .ttype = FULL_QUEUE,
	 .non_block = 1,
	 .len = 16,
	 .ret = -1,
	 .err = EAGAIN,
	 },
	{			// case09
	 .ttype = NORMAL,
	 .len = 1,
	 .prio = 32768,		// max priority + 1
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			// case10
	 .ttype = FULL_QUEUE,
	 .len = 16,
	 .sec = -1,
	 .nsec = 0,
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			// case11
	 .ttype = FULL_QUEUE,
	 .len = 16,
	 .sec = 0,
	 .nsec = -1,
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			// case12
	 .ttype = FULL_QUEUE,
	 .len = 16,
	 .sec = 0,
	 .nsec = 1000000000,
	 .ret = -1,
	 .err = EINVAL,
	 },
	{			// case13
	 .ttype = FULL_QUEUE,
	 .len = 16,
	 .sec = 0,
	 .nsec = 999999999,
	 .ret = -1,
	 .err = ETIMEDOUT,
	 },
	{			// case14
	 .ttype = SEND_SIGINT,
	 .len = 16,
	 .ret = -1,
	 .sec = 3,
	 .nsec = 0,
	 .err = EINTR,
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
	int oflag;
	int i, rc, cmp_ok = 1, fd = -1;
	char smsg[MAX_MSGSIZE], rmsg[MAX_MSGSIZE];
	struct timespec ts = { 0, 0 };
	pid_t pid = 0;
	unsigned prio;

	/*
	 * When test ended with SIGTERM etc, mq descriptor is left remains.
	 * So we delete it first.
	 */
	TEST(mq_unlink(QUEUE_NAME));

	switch (tc->ttype) {
	case FD_NOT_EXIST:
		fd = INT_MAX - 1;
		/* fallthrough */
	case FD_NONE:
		break;
	case FD_FILE:
		TEST(fd = open("/", O_RDONLY));
		if (fd < 0) {
			tst_resm(TFAIL, "can't open \"/\".- errno = %d : %s\n",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			result = 1;
			goto EXIT;
		}
		break;
	default:
		/*
		 * Open message queue
		 */
		oflag = O_CREAT | O_EXCL | O_RDWR;
		if (tc->non_block)
			oflag |= O_NONBLOCK;

		TEST(fd = mq_open(QUEUE_NAME, oflag, S_IRWXU, NULL));
		if (TEST_RETURN < 0) {
			tst_resm(TFAIL, "mq_open failed - errno = %d : %s\n",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			result = 1;
			goto EXIT;
		}
		if (tc->ttype == FULL_QUEUE || tc->ttype == SEND_SIGINT) {
			for (i = 0; i < MAX_MSG; i++) {
				TEST(rc =
				     mq_timedsend(fd, smsg, tc->len, 0, &ts));
				if (rc < 0) {
					tst_resm(TFAIL,
						 "mq_timedsend failed - errno = %d : %s\n",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
					result = 1;
					goto EXIT;
				}
			}
			if (tc->ttype == SEND_SIGINT) {
				pid = create_sig_proc(200000, SIGINT, UINT_MAX);
				if (pid < 0) {
					result = 1;
					goto EXIT;
				}
			}
		}
		break;
	}

	/*
	 * Prepare send message
	 */
	for (i = 0; i < tc->len && i < sizeof(smsg); i++)
		smsg[i] = i;

	/*
	 * Set the timeout value
	 */
	ts.tv_sec = tc->sec;
	ts.tv_nsec = tc->nsec;
	if (tc->sec >= 0 || tc->nsec != 0)
		ts.tv_sec += time(NULL);

	/*
	 * Execut test system call
	 */
	errno = 0;
	TEST(sys_ret = mq_timedsend(fd, smsg, tc->len, tc->prio, &ts));
	sys_errno = errno;
	if (sys_ret < 0)
		goto TEST_END;

	/*
	 * Receive echoed message and compare
	 */
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	TEST(rc = mq_timedreceive(fd, rmsg, MAX_MSGSIZE, &prio, &ts));
	if (rc < 0) {
		tst_resm(TFAIL, "mq_timedreceive failed - errno = %d : %s\n",
			 TEST_ERRNO, strerror(TEST_ERRNO));
		result = 1;
		goto EXIT;
	}
	if (rc != tc->len || tc->prio != prio)
		cmp_ok = 0;
	else {
		for (i = 0; i < tc->len; i++)
			if (rmsg[i] != smsg[i]) {
				cmp_ok = 0;
				break;
			}
	}
TEST_END:
	/*
	 * Check results
	 */
	result |= (sys_errno != tc->err) || !cmp_ok;
	PRINT_RESULT_CMP(sys_ret >= 0, tc->ret, tc->err, sys_ret, sys_errno,
			 cmp_ok);

EXIT:
	if (fd >= 0) {
		TEST(close(fd));
		TEST(mq_unlink(QUEUE_NAME));
	}
	if (pid > 0) {
		int st;
		kill(pid, SIGTERM);
		wait(&st);
	}
	return result;
}

/*
 * main()
 */

int main(int ac, char **av)
{
	int result = RESULT_OK;
	int i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			/*
			 * Execute test
			 */
			for (i = 0; i < (int)ARRAY_SIZE(tcase); i++) {
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
			switch (result) {
			case RESULT_OK:
				tst_resm(TPASS, "mq_timedsend call succeeded");
				break;

			default:
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "mq_timedsend failed");
			}

		}
	}
	cleanup();
	tst_exit();
}
