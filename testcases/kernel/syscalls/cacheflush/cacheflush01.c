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
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
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

#include "test.h"
#include "lapi/syscalls.h"

#if __NR_cacheflush != __LTP__NR_INVALID_SYSCALL
#include <asm/cachectl.h>
#else
#ifndef   ICACHE
#define   ICACHE   (1<<0)	/* flush instruction cache        */
#endif
#ifndef   DCACHE
#define   DCACHE   (1<<1)	/* writeback and flush data cache */
#endif
#ifndef   BCACHE
#define   BCACHE   (ICACHE|DCACHE)	/* flush both caches              */
#endif
#endif

char *TCID = "cacheflush01";
int TST_TOTAL = 1;

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
void cleanup(void)
{

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
void setup(void)
{
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av)
{

	char *addr = NULL;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	tst_count = 0;
	/* Create some user address range */
	addr = malloc(getpagesize());
	if (addr == NULL) {
		tst_brkm(TFAIL | TTERRNO, cleanup, "malloc failed");
	}

	/* Invokes cacheflush() with proper parameters */
	TEST(ltp_syscall(__NR_cacheflush, addr, getpagesize(), ICACHE));
	if (TEST_RETURN == 0) {
		tst_resm(TPASS, "passed with no errno");
	} else {
		tst_resm(TFAIL, "failed with unexpected errno");
	}

	TEST(ltp_syscall(__NR_cacheflush, addr, getpagesize(), DCACHE));
	if (TEST_RETURN == 0) {
		tst_resm(TPASS, "passed with no errno");
	} else {
		tst_resm(TFAIL, "failed with unexpected errno");
	}

	TEST(ltp_syscall(__NR_cacheflush, addr, getpagesize(), BCACHE));
	if (TEST_RETURN == 0) {
		tst_resm(TPASS, "passed with no errno");
	} else {
		tst_resm(TFAIL, "failed with unexpected errno");
	}

	cleanup();
	tst_exit();
}
