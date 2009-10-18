/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd                */
/*          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,	      */
/*		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>, 	      */
/*		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>	      */
/*                                                                  	      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        clock_nanosleep01.c                                           */
/*                                                                            */
/* Description: This tests the clock_nanosleep() syscall                      */
/*									      */
/* 									      */
/*									      */
/*									      */
/*									      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* clock_nanosleep01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   clock_nanosleep01                                             */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <sys/syscall.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include "../utils/common_j_h.c"
#include "../utils/include_j_h.h"

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "clock_nanosleep01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 1;                   /* total number of tests in this file.   */
struct sigaction act;

/*
 * sighandler()
 */
void sighandler(int sig)
{
        if (sig == SIGINT)
                return;
        // NOTREACHED
        return;
}

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void cleanup() {
        /* Remove tmp dir and all files in it */
        TEST_CLEANUP;
        tst_rmdir();

        /* Exit with appropriate return code. */
        tst_exit();
}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup() {
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
#define SYSCALL_NAME    "clock_nanosleep"


/*
 * Global variables
 */
static int opt_debug;
static char *progname;

enum test_type {
		NORMAL,
		NULL_POINTER,
		SEND_SIGINT,
};


/*
 * Data Structure
 */
struct test_case {
	clockid_t clk_id;
	int ttype;
        int flags;
        time_t sec;
        long nsec;
	int ret;
        int err;
};


/* Test cases
 *
 *   test status of errors on man page
 *
 *   EINTR              v (function was interrupted by a signal)
 *   EINVAL             v (invalid tv_nsec, etc.)
 *   ENOTSUP            can't check because not supported clk_id generates
 *                      EINVAL
 */


static struct test_case tcase[] = {
        { // case00
                .clk_id         = CLOCK_REALTIME,
                .ttype          = NORMAL,
		.flags          = 0,
                .sec            = 0,
                .nsec           = 500000000, // 500msec
                .ret            = 0,
                .err            = 0,
        },
        { // case01
                .clk_id         = CLOCK_MONOTONIC,
                .ttype          = NORMAL,
		.flags          = 0,
                .sec            = 0,
                .nsec           = 500000000, // 500msec
                .ret            = 0,
                .err            = 0,
        },
	{ // case02
                .ttype          = NORMAL,
                .clk_id         = CLOCK_REALTIME,
                .flags          = 0,
                .sec            = 0,
                .nsec           = -1,   // invalid
                .ret            = EINVAL,
                .err            = 0,
        },
        { // case03
                .ttype          = NORMAL,
                .clk_id         = CLOCK_REALTIME,
                .flags          = 0,
                .sec            = 0,
                .nsec           = 1000000000,   // invalid
                .ret            = EINVAL,
                .err            = 0,
        },
        { // case04
                .ttype          = NORMAL,
                .clk_id         = CLOCK_THREAD_CPUTIME_ID, // not supported
                .flags          = 0,
                .sec            = 0,
                .nsec           = 500000000, // 500msec
                .ret            = EINVAL, // RHEL4U1 + 2.6.18 returns EINVAL
                .err            = 0,
        },
        { // case05
                .ttype          = SEND_SIGINT,
                .clk_id         = CLOCK_REALTIME,
                .flags          = 0,
                .sec            = 10,
                .nsec           = 0,
                .ret            = EINTR,
                .err            = 0,
        },
#if 0   // glibc generates SEGV error (RHEL4U1 + 2.6.18)
        { // caseXX
                .ttype          = NULL_POINTER,
                .clk_id         = CLOCK_REALTIME,
                .flags          = 0,
                .sec            = 0,
                .nsec           = 500000000, // 500msec
                .ret            = EFAULT,
                .err            = 0,
        },
#endif
};


/*
 * chk_difftime()
 *   Return : OK(0), NG(-1)
 */
#define MAX_MSEC_DIFF   20

static int chk_difftime(struct timespec *bef, struct timespec *aft,
                        time_t sec, long nsec)
{
        struct timespec t;
        time_t expect;
        time_t result;

        t.tv_sec = aft->tv_sec - bef->tv_sec;
        t.tv_nsec = aft->tv_nsec - bef->tv_nsec;
        if (t.tv_nsec < 0) {
                t.tv_sec -= 1;
                t.tv_nsec += 1000000000;
        }
        expect = (sec * 1000) + (nsec / 1000000);
        result = (t.tv_sec * 1000) + (t.tv_nsec / 1000000);
        tst_resm(TINFO,"check sleep time: (min:%ld) < %ld < (max:%ld) (msec)",expect - MAX_MSEC_DIFF, result, expect + MAX_MSEC_DIFF);
        if (result < expect - MAX_MSEC_DIFF || result > expect + MAX_MSEC_DIFF)
                return -1;
	return 0;
}


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
        struct timespec beftp, afttp, rq, rm;
        int rc, range_ok = 1, remain_ok = 1;
        pid_t pid = 0;

	 /*
         * Check before sleep time
         */
        if (tc->ttype == SEND_SIGINT) {
                pid = create_sig_proc(500000, SIGINT, UINT_MAX);
                if (pid < 0)
                        return 1;
        }

        /*
         * Check before sleep time
         */
        TEST(rc = clock_gettime(tc->clk_id, &beftp));
        if (rc < 0) {
                tst_resm(TFAIL|TTERRNO, "iclock_gettime failed");
                result = 1;
                goto EXIT;
        }
	 /*
         * Execute system call
         */
        rq.tv_sec = tc->sec;
        rq.tv_nsec = tc->nsec;
        // !!!CAUTION: 'clock_gettime' returns errno itself
        errno = 0;
        if (tc->ttype == NULL_POINTER)
                TEST(sys_ret = clock_nanosleep(tc->clk_id, tc->flags, NULL, &rm));
        else
                TEST(sys_ret = clock_nanosleep(tc->clk_id, tc->flags, &rq, &rm));
        sys_errno = errno;
        if (sys_ret != 0)
                goto TEST_END;
	
	 /*
         * Check after sleep time
         */
        TEST(rc = clock_gettime(tc->clk_id, &afttp));
        if (TEST_RETURN < 0) {
                EPRINTF("clock_gettime failed.\n");
                result = 1;
                goto EXIT;
        }
	range_ok = chk_difftime(&beftp, &afttp, tc->sec, tc->nsec) == 0;
	/*
         * Check remaining time
         */
TEST_END:
        if (tc->ttype == NORMAL || tc->ttype == SEND_SIGINT) {
                tst_resm(TINFO,"remain time: %ld %ld", rm.tv_sec, rm.tv_nsec);
                if (tc->ttype == NORMAL)
                        remain_ok = 1;
                else
                        remain_ok = rm.tv_sec != 0 || rm.tv_nsec != 0;
        }

	/*
         * Check results
         */
        result |= (sys_ret != tc->ret) || !range_ok || !remain_ok;
        if (!range_ok)
                PRINT_RESULT_EXTRA(0, tc->ret, tc->err, sys_ret, sys_errno,"time range check", range_ok);
        else
                PRINT_RESULT_EXTRA(0, tc->ret, tc->err, sys_ret, sys_errno,"remain time check", remain_ok);
EXIT:
        if (pid > 0) {
                int st;
                TEST(kill(pid, SIGTERM));
                TEST(wait(&st));
        }
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
        tst_resm(TINFO,"    -d --debug           Show debug messages");
        tst_resm(TINFO,"    -h --help            Show this message");
        tst_resm(TINFO,"NG");
        exit(1);
}


/*
 * main()
 */



int main(int ac, char **av) {
	int result = RESULT_OK;
        int c;
        int i;
        int lc;                 /* loop counter */
        char *msg;              /* message returned from parse_opts */

	struct option long_options[] = {
                { "debug", no_argument, 0, 'd' },
                { "help",  no_argument, 0, 'h' },
                { NULL, 0, NULL, 0 }
        };

	progname = strchr(av[0], '/');
        progname = progname ? progname + 1 : av[0];	
	
        /* parse standard options */
        if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
             tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
             tst_exit();
           }

        setup();

        /* Check looping state if -i option given */
        for (lc = 0; TEST_LOOPING(lc); ++lc) {
                Tst_count = 0;
                for (testno = 0; testno < TST_TOTAL; ++testno) {
			 TEST(c = getopt_long(ac, av, "dh", long_options, NULL));
			 while(TEST_RETURN != -1) {
		                switch (c) {
                		case 'd':
		                        opt_debug = 1;
                		        break;
		                default:
                		        usage(progname);
                        		// NOTREACHED
                		}
		        }


		if (ac != optind) {
        	        tst_resm(TINFO,"Options are not match.");
                	usage(progname);
                	// NOTREACHED
	        }

		/*
		* Execute test
         	*/
	        for (i = 0; i < (int)(sizeof(tcase) / sizeof(tcase[0])); i++) {
        	        int ret;
	                tst_resm(TINFO,"(case%02d) START", i);
	                ret = do_test(&tcase[i]);
	                tst_resm(TINFO,"(case%02d) END => %s", i, (ret == 0) ? "OK" : "NG");
	                result |= ret;
        	}
		
		/*
        	 * Check results
         	*/
        	switch(result) {
	        case RESULT_OK:
        			tst_resm(TPASS, "clock_nanosleep call succeeded");
		                break;

	        default:
                 	   	tst_resm(TFAIL, "clock_nanosleep failed");
        		        tst_resm(TINFO,"NG");
				cleanup();
				tst_exit();
		                break;
        	}

                }
        }	
        cleanup();
	tst_exit();
}

