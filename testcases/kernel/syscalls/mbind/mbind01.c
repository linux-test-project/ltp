/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008			      */
/* Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,		      */
/*	    Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,		      */
/*	    Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>  		      */
/*									      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or	      */
/* (at your option) any later version.					      */
/*									      */
/* This program is distributed in the hope that it will be useful,	      */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		      */
/* the GNU General Public License for more details.			      */
/*									      */
/* You should have received a copy of the GNU General Public License	      */
/* along with this program;  if not, write to the Free Software	              */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									      */
/******************************************************************************/
/******************************************************************************/
/*									      */
/* File:	mbind01.c					              */
/*									      */
/* Description: This tests the mbind() syscall		                      */
/*									      */
/* Usage:  <for command-line>						      */
/* mbind01 [-c n] [-e][-i n] [-I x] [-p x] [-t]		                      */
/*      where,  -c n : Run n copies concurrently.			      */
/*	      -e   : Turn on errno logging.				      */
/*	      -i n : Execute test n times.				      */
/*	      -I x : Execute test for x seconds.			      */
/*	      -P x : Pause for x seconds between iterations.		      */
/*	      -t   : Turn on syscall timing.				      */
/*									      */
/* Total Tests: 1							      */
/*									      */
/* Test Name:   mbind01					                      */
/* History:     Porting from Crackerjack to LTP is done by		      */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			      */
/******************************************************************************/

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Extern Global Variables */
extern int Tst_count;		/* counter for tst_xxx routines.	 */
extern char *TESTDIR;		/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "mbind01";		/* Test program identifier.*/
int  testno;
int  TST_TOTAL = 2;		/* total number of tests in this file.   */

#include "config.h"

#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H && HAVE_MPOL_CONSTANTS
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "include_j_h.h"
#include "numa_helpers.h"
#include <numaif.h>

#if ! defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2

/* Extern Global Functions */
/******************************************************************************/
/*									      */
/* Function:    cleanup						              */
/*									      */
/* Description: Performs all one time clean up for this test on successful    */
/*	      completion,  premature exit or  failure. Closes all temporary   */
/*	      files, removes all temporary directories exits the test with    */
/*	      appropriate return code by calling tst_exit() function.         */
/*									      */
/* Input:       None.							      */
/*									      */
/* Output:      None.							      */
/*									      */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*	      On success - Exits calling tst_exit(). With '0' return code.    */
/*									      */
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
/*									      */
/* Function:    setup							      */
/*									      */
/* Description: Performs all one time setup for this test. This function is   */
/*	      typically used to capture signals, create temporary dirs        */
/*	      and temporary files that may be used in the course of this      */
/*	      test.							      */
/*									      */
/* Input:       None.							      */
/*									      */
/* Output:      None.							      */
/*									      */
/* Return:      On failure - Exits by calling cleanup().		      */
/*	      On success - returns 0.				              */
/*									      */
/******************************************************************************/
void setup() {
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int sig_count = 0;

void sig_action(int sig) {
	sig_count = 1;
}

/*
 * Macros
 */
#define SYSCALL_NAME    "mbind"

/*
 * Global variables
 */
static int opt_debug;
static char *progname;

enum test_type {
	NORMAL,
	INVALID_POINTER,
};

enum from_node {
	NONE,
	SELF,
};

/*
 * Data Structure
 */
struct test_case {
	int ttype;
	int policy;
	int from_node;
	unsigned flags;
	int ret;
	int err;
};

/* Test cases
 *
 *   test status of errors on man page
 *
 *   EFAULT	     v (detect unmapped hole or invalid pointer)
 *   EINVAL	     v (invalid arguments)
 *   ENOMEM	     can't check because it's difficult to create no-memory
 *   EIO		can't check because we don't have N-node NUMA system
 *		      (only we can do is simulate 1-node NUMA)
 */

static struct test_case tcase[] = {
	{ /* case00 */
		.policy	    = MPOL_DEFAULT,
		.from_node  = NONE,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case01 */
		.policy	    = MPOL_DEFAULT,
		.from_node  = SELF, /* target exists */
		.ret	    = -1,
		.err	    = EINVAL,
	},
	{ /* case02 */
		.policy	    = MPOL_BIND,
		.from_node  = NONE, /* no target */
		.ret	    = -1,
		.err	    = EINVAL,
	},
	{ /* case03 */
		.policy	    = MPOL_BIND,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case04 */
		.policy	    = MPOL_INTERLEAVE,
		.from_node  = NONE, /* no target */
		.ret	    = -1,
		.err	    = EINVAL,
	},
	{ /* case05 */
		.policy	    = MPOL_INTERLEAVE,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case06 */
		.policy	    = MPOL_PREFERRED,
		.from_node  = NONE,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case07 */
		.policy	    = MPOL_PREFERRED,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case08 */
		.policy	    = -1, /* unknown policy */
		.from_node  = NONE,
		.ret	    = -1,
		.err	    = EINVAL,
	},
	{ /* case09 */
		.policy	    = MPOL_DEFAULT,
		.from_node  = NONE,
		.flags	    = -1, // invalid flags */
		.ret	    = -1,
		.err	    = EINVAL,
	},
	{ // case10 */
		.ttype	    = INVALID_POINTER,
		.policy	    = MPOL_PREFERRED,
		.from_node  = SELF,
		.ret	    = -1,
		.err	    = EFAULT,
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

static int do_test(struct test_case *tc) {
	int sys_ret;
	int sys_errno;
	int result = RESULT_OK;
	int rc, policy, cmp_ok = 1;
	char *p = NULL;
	nodemask_t nodemask, getnodemask;
	unsigned long maxnode = NUMA_NUM_NODES;
	unsigned long len = MEM_LENGTH;
	unsigned long *invalid_nodemask;

	/* We assume that there is only one node(node0). */
	nodemask_zero(&nodemask);
	nodemask_set(&nodemask, 0);
	nodemask_zero(&getnodemask);

	/*
	 * mmap memory
	 */
	p = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,0, 0);
	if (p == (void*)-1) {
		tst_resm(TFAIL | TERRNO, "malloc failed");
		cleanup();
		tst_exit();
	}
	if(tc->ttype == INVALID_POINTER)
		invalid_nodemask = (unsigned long *)0xc0000000;
	/*
	 * Execute system call
	 */
	errno = 0;
	if (tc->from_node == NONE)
		TEST(sys_ret = syscall(__NR_mbind, p, len, tc->policy, NULL, 0,
					tc->flags));
	else if (tc->ttype == INVALID_POINTER)
		TEST(sys_ret = syscall(__NR_mbind, p, len, tc->policy,
					invalid_nodemask, maxnode, tc->flags));
	else
		TEST(sys_ret = syscall(__NR_mbind, p, len, tc->policy,&nodemask,
					maxnode, tc->flags));
	sys_errno = errno;
	if (sys_ret < 0)
		goto TEST_END;

	/*
	 * Check policy of the allocated memory
	 */
	TEST(rc = syscall(__NR_get_mempolicy, &policy, &getnodemask, maxnode,p, MPOL_F_ADDR));
	if (rc < 0) {
		tst_resm(TFAIL | TERRNO, "get_mempolicy failed");
		result = 1;
		tst_exit();
	}

	/* When policy equals MPOL_DEFAULT, then get_mempolicy not return node */
	if (tc->policy == MPOL_DEFAULT)
		nodemask_zero(&nodemask);
	if ((tc->policy == MPOL_PREFERRED) && (tc->from_node == NONE))
		cmp_ok = (tc->policy == policy);
	else
		cmp_ok = ((tc->policy == policy) && nodemask_equal(&nodemask, &getnodemask));
	if (opt_debug) {
		nodemask_dump("nodemask Expect:", &nodemask);
		nodemask_dump("nodemask Result:", &getnodemask);
		tst_resm(TINFO, "policy E:%d R:%d", tc->policy, policy);
	}


TEST_END:
	/*
	 * Check results
	 */
	result |= ((sys_errno != tc->err) || (!cmp_ok));
	PRINT_RESULT_CMP(0, tc->ret, tc->err, sys_ret, sys_errno, cmp_ok);
	return result;
}

/*
 * usage()
 */
static void usage(char *progname) {
	tst_resm(TINFO,"usage: %s [options]", progname);
	tst_resm(TINFO,"This is a regression test program of %s system call.",SYSCALL_NAME);
	tst_resm(TINFO,"options:");
	tst_resm(TINFO,"    -d --debug	   Show debug messages");
	tst_resm(TINFO,"    -h --help	    Show this message");
	tst_resm(TINFO,"NG");
	exit(1);
}

int main(int argc, char **argv) {
	char *msg;	      /* message returned from parse_opts */

	struct option long_options[] = {
		{ "debug", no_argument, 0, 'd' },
		{ "help",  no_argument, 0, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	
	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *)NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	progname = basename(argv[0]);

	setup();

	int lc, i, ret;		 /* loop counter */

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		Tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; testno++) {
			TEST(getopt_long(argc, argv, "dh", long_options, NULL));
			while (TEST_RETURN != -1){
				switch (TEST_RETURN) {
				case 'd':
					opt_debug = 1;
					break;
				default:
					usage(progname);
				}
			} /* end of while */
			if(argc != optind) {
				tst_resm(TFAIL | TERRNO, "Options don't match");
				usage(progname);
				cleanup();
				tst_exit();
			}
			
			/*
			 * Execute test
			 */
			for (i = 0; i < (int)(sizeof(tcase) / sizeof(tcase[0])); i++) {
				tst_resm(TINFO,"(case%02d) START", i);
				ret = do_test(&tcase[i]);
				tst_resm((ret == 0 ? TPASS : TFAIL | TERRNO), "(case%02d) END", i);
			}

		}

	}
	cleanup();
	tst_exit();
}
#else /* libnuma v2 */
int main(void) {
	tst_resm(TBROK, "XXX: test is broken on libnuma v2 (read numa_helpers.h for more details).");
	return 0;
}
#endif
#else /* no numaif.h // numa.h */
int main(void) {
	tst_resm(TCONF, "System doesn't have required numa support");
	tst_exit();
}
#endif
