/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/* Copyright 2008 Paul Mackerras, IBM Corp.                                   */
/*                                                                            */
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
/* File:        endian_switch01.c                                                    */
/*                                                                            */
/* Description: Test little-endian mode switch system call. Requires a 64-bit */
/*              processor that supports little-endian mode,such as POWER6.    */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   endian_switch01                                                      */
/*                                                                            */
/* Author:      Paul Mackerras <paulus@samba.org>                             */
/*                                                                            */
/* History:     Created - Sep 02 2008 - Paul Mackerras <paulus@samba.org>     */
/*              Ported to LTP                                                 */
/*                      - Sep 02 2008                                         */
/*                      - Subrata Modak <subrata@linux.vnet.ibm.com>          */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <signal.h>
#include <setjmp.h>
#include "test.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <unistd.h>
#include "linux_syscall_numbers.h"

#if defined (__powerpc64__) || (__powerpc__)
static void setup();
#endif

static void cleanup();

char *TCID = "endian_switch01";
int TST_TOTAL = 1;

#if defined (__powerpc64__) || (__powerpc__)
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

extern int main4(int ac, char **av, char **envp, unsigned long *auxv)
__asm("main");
#endif

void cleanup(void)
{

}

#if defined (__powerpc64__) || (__powerpc__)
#ifndef PPC_FEATURE_TRUE_LE
#define PPC_FEATURE_TRUE_LE              0x00000002
#endif

#include <asm/cputable.h>

volatile int got_sigill;
sigjmp_buf jb;

void sigill(int sig)
{
	got_sigill = 1;
	siglongjmp(jb, 1);
}

void do_le_switch(void)
{
	register int r0 asm("r0");

	r0 = 0x1ebe;
	asm volatile ("sc; .long 0x02000044":"=&r" (r0):"0"(r0)
		      :"cr0", "r9", "r10", "r11", "r12");
}

int main4(int ac, char **av, char **envp, unsigned long *auxv)
{

	if ((tst_kvercmp(2, 6, 26)) < 0) {
		tst_brkm(TCONF,
			 NULL,
			 "This test can only run on kernels that are 2.6.26 and higher");
	}
	setup();
	for (; *auxv != AT_NULL && *auxv != AT_HWCAP; auxv += 2) ;
	if (!(auxv[0] == AT_HWCAP && (auxv[1] & PPC_FEATURE_TRUE_LE))) {
		tst_brkm(TCONF, cleanup,
			 "Processor does not support little-endian mode");
	}
	signal(SIGILL, sigill);
	if (sigsetjmp(jb, 1) == 0)
		do_le_switch();
	if (got_sigill) {
		tst_brkm(TFAIL, NULL, "Got SIGILL - test failed");
	}
	tst_resm(TPASS, "endian_switch() syscall tests passed");
	tst_exit();
}

#else

int main(void)
{

	tst_brkm(TCONF, cleanup,
		 "This system does not support running of switch() syscall");
}

#endif
