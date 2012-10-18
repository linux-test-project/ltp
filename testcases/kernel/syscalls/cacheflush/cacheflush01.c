/******************************************************************************/
/* Copyright (c) Maxin John <maxin.john@gmail.com>, 2009                      */
/* LKML Reference: http://lkml.org/lkml/2009/4/9/203                          */
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
/* File:        cacheflush01.c                                                */
/*                                                                            */
/* Description: The cacheflush_check() syscall                     	      */
/*		Tests EINVAL error of cacheflush system call.		      */
/* 		Its expected behaviour is cacheflush() should return -EINVAL  */
/*		when cache parameter is not one of ICACHE, DCACHE, or BCACHE. */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* cacheflush01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                          */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   cacheflush01                                                  */
/******************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#if defined __NR_cacheflush && __NR_cacheflush > 0
#include <asm/cachectl.h>
#else
/* Fake linux_syscall_numbers.h */
#define __NR_cacheflush		0
#ifndef   ICACHE
#define   ICACHE   (1<<0)		/* flush instruction cache        */
#endif
#ifndef   DCACHE
#define   DCACHE   (1<<1)		/* writeback and flush data cache */
#endif
#ifndef   BCACHE
#define   BCACHE   (ICACHE|DCACHE)	/* flush both caches              */
#endif
#endif

/* Extern Global Variables */

/* Global Variables */
char *TCID = "cacheflush01";	/* Test program identifier.*/
int  TST_TOTAL = 1;		/* total number of tests in this file.   */

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

        TEST_CLEANUP;
        tst_rmdir();
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
        /* Create temporary directories */
        TEST_PAUSE;
        tst_tmpdir();
}

int main(int ac, char **av)
{

	char *addr = NULL;
        char *msg;              /* message returned from parse_opts */

        /* parse standard options */
        if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
        }

        setup();

	Tst_count = 0;
	/* Create some user address range */
	addr = malloc(getpagesize());
	if (addr == NULL) {
		tst_brkm(TFAIL | TTERRNO, cleanup, "malloc failed");
	}

	/* Invokes cacheflush() with proper parameters */
	TEST(syscall(__NR_cacheflush, addr, getpagesize(), ICACHE));
	if (TEST_RETURN == 0) {
		tst_resm(TPASS, "passed with no errno");
	} else {
		tst_resm(TFAIL, "failed with unexpected errno");
	}

	TEST(syscall(__NR_cacheflush, addr, getpagesize(), DCACHE));
	if (TEST_RETURN == 0) {
		tst_resm(TPASS, "passed with no errno");
	} else {
		tst_resm(TFAIL, "failed with unexpected errno");
	}

	TEST(syscall(__NR_cacheflush, addr, getpagesize(), BCACHE));
	if (TEST_RETURN == 0) {
		tst_resm(TPASS, "passed with no errno");
	} else {
		tst_resm(TFAIL, "failed with unexpected errno");
	}

	cleanup();
        tst_exit();
}
