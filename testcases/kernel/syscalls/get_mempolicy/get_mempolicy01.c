/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd		      */
/*	  Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,	      */
/*		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>, 	      */
/*		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>	      */
/*								  	      */
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
/* File:	get_mempolicy01.c					      */
/*									      */
/* Description: This tests the get_mempolicy() syscall			      */
/*									      */
/* 									      */
/*									      */
/*									      */
/*									      */
/*									      */
/* Usage:  <for command-line>						      */
/* get_mempolicy01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			      */
/*      where,  -c n : Run n copies concurrently.			      */
/*	      -e   : Turn on errno logging.				      */
/*	      -i n : Execute test n times.				      */
/*	      -I x : Execute test for x seconds.			      */
/*	      -P x : Pause for x seconds between iterations.		      */
/*	      -t   : Turn on syscall timing.				      */
/*									      */
/* Total Tests: 1							      */
/*									      */
/* Test Name:   get_mempolicy01					              */
/* History:     Porting from Crackerjack to LTP is done by		      */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			      */
/******************************************************************************/

/* Harness Specific Include Files. */
#include "usctest.h"
#include "test.h"
#include "config.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "get_mempolicy01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 1;		   /* total number of tests in this file.   */

#if HAVE_NUMA_H && HAVE_NUMAIF_H && HAVE_MPOL_CONSTANTS
#include <sys/types.h>
#include <sys/mman.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <numa.h>
#include <numaif.h>
#include "linux_syscall_numbers.h"
#include "include_j_h.h"
#include "common_j_h.c"
#include "numa_helpers.h"
#if ! defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2

/*
 * Macros
 */
#define SYSCALL_NAME    "get_mempolicy"

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
 * Global variables
 */
static char *progname;

enum test_type {
	DEFAULT,	/* get default policy */
	ADDR,		/* get policy of memory which include mapped address */
	INVALID_POINTER,
	INVALID_FLAGS,
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
	int ret;
	int err;
};

/* Test cases
 *
 *   test status of errors on man page
 *
 *   (NONE)	     man page hadn't been completed.
 *
 *   test status of errors NOT on man page
 *
 *   EFAULT	     v (invalid address)
 *   EINVAL	     v (invalid parameters)
 */

static struct test_case tcase[] = {
	{ /* case00 */
		.ttype	    = DEFAULT,
		.policy	    = MPOL_DEFAULT,
		.from_node  = NONE,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case01 */
		.ttype	    = DEFAULT,
		.policy	    = MPOL_BIND,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case02 */
		.ttype	    = DEFAULT,
		.policy	    = MPOL_INTERLEAVE,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
{ /* case03 */
		.ttype	    = DEFAULT,
		.policy	    = MPOL_PREFERRED,
		.from_node  = NONE,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case04 */
		.ttype	    = DEFAULT,
		.policy	    = MPOL_PREFERRED,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case05 */
		.ttype	    = ADDR,
		.policy	    = MPOL_DEFAULT,
		.from_node  = NONE,
		.ret	    = 0,
		.err	    = 0,
	},
{ /* case06 */
		.ttype	    = ADDR,
		.policy	    = MPOL_BIND,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case07 */
		.ttype	    = ADDR,
		.policy	    = MPOL_INTERLEAVE,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case08 */
		.ttype	    = ADDR,
		.policy	    = MPOL_PREFERRED,
		.from_node  = NONE,
		.ret	    = 0,
		.err	    = 0,
	},
{ /* case09 */
		.ttype	    = ADDR,
		.policy     = MPOL_PREFERRED,
		.from_node  = SELF,
		.ret	    = 0,
		.err	    = 0,
	},
	{ /* case10 */
		.ttype	    = INVALID_POINTER,
		.policy	    = MPOL_DEFAULT,
		.from_node  = NONE,
		.ret	    = -1,
		.err	    = EFAULT,
	},
	{ /* case11 */
		.ttype	    = INVALID_FLAGS,
		.policy	    = MPOL_DEFAULT,
		.from_node  = NONE,
		.ret	    = -1,
		.err	    = EINVAL,
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

static int do_test (struct test_case *tc)
{
	int sys_ret;
	int sys_errno;
	int result = RESULT_OK;
	int rc, policy, flags, cmp_ok = 1;

/*
#if LIBNUMA_API_VERSION == 2
	struct bitmask nodemask, getnodemask;
#else
 */
	nodemask_t nodemask, getnodemask;
/*
#endif
 */
	unsigned long maxnode = NUMA_NUM_NODES;
	char *p = NULL;
	unsigned long len = MEM_LENGTH;

	/* We assume that there is only one node(node0). */
	nodemask_zero(&nodemask); /* Segfaults here with libnuma v2. */
	nodemask_set(&nodemask, 0);
	nodemask_zero(&getnodemask);

	switch (tc->ttype) {
	case DEFAULT:
		flags = 0;
		p = NULL;
		/* set memory policy */
		if (tc->from_node == NONE)
			TEST(rc = syscall(__NR_set_mempolicy, tc->policy, NULL, 0));
		else
			TEST(rc = syscall(__NR_set_mempolicy, tc->policy, &nodemask, maxnode));

		if (TEST_RETURN < 0) {
			tst_resm(TFAIL|TTERRNO, "set_mempolicy() failed");
			result = 1;
			cleanup();
			tst_exit();
		}
		break;
	default:
		flags = MPOL_F_ADDR;
		/* mmap memory */
		p = mmap(NULL, len, PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
		if (p == (void*)-1) {
			tst_resm(TFAIL|TERRNO, "mmap() failed");
			result = 1;
			cleanup();
			tst_exit();
		}
		/* set memory policy */
		if (tc->from_node == NONE)
			TEST(rc = syscall(__NR_mbind, p, len, tc->policy,NULL, 0, 0));
		else
			TEST(rc = syscall(__NR_mbind, p, len, tc->policy,&nodemask, maxnode, 0));
		if (tc->ttype == INVALID_POINTER)
			p = NULL;

		if (tc->ttype == INVALID_FLAGS) {
			flags = -1;
			break;
		}
	}
	/*
	 * Execute system call
	 */
	errno = 0;
	TEST(sys_ret = syscall(__NR_get_mempolicy, &policy, &getnodemask, maxnode,p, flags));
	sys_errno = errno;
	if (sys_ret < 0)
		goto TEST_END;

	// When policy equals MPOL_DEFAULT, then get_mempolicy not return node
	if (tc->policy == MPOL_DEFAULT)
		nodemask_zero(&nodemask);
	cmp_ok = tc->policy == policy && (tc->from_node == NONE || nodemask_equal(&nodemask,&getnodemask));
TEST_END:
	/*
	 * Check results
	 */
	result |= (sys_errno != tc->err) || !cmp_ok;
	PRINT_RESULT_CMP(0, tc->ret, tc->err, sys_ret, sys_errno, cmp_ok);
	return result;
}

int main(int argc, char **argv) {
	int i, ret;

	setup();

	ret = 0;

	/*
	 * Execute test
	 */
	for (i = 0; ret == 0 && i < (int)(sizeof(tcase) / sizeof(*tcase));
	    i++) {
		tst_resm(TINFO, "(case%02d) START", i);
		ret = do_test(&tcase[i]);
		tst_resm((ret == 0 ? TPASS : TFAIL|TERRNO), "(case%02d) END",
		    i);
	}

	cleanup();
	tst_exit();

}
#else
int main(void) {
	tst_brkm(TCONF, NULL, "XXX: test is broken on libnuma v2 (read numa_helpers.h for more details).");
	tst_exit();
}
#endif
#else
int main(void) {
	tst_brkm(TCONF, NULL, "System doesn't have required numa support");
	tst_exit();
}
#endif
