/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: f00f.c,v 1.5 2008/04/24 06:36:14 subrata_modak Exp $ */
/*
 * This is a simple test for handling of the pentium f00f bug.
 * It is an example of a catistrophic test case.  If the system
 * doesn't correctly handle this test, it will likely lockup.
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"

char *TCID = "f00f";
int TST_TOTAL = 1;

#ifdef __i386__

void sigill(int sig)
{
	tst_resm(TPASS, "SIGILL received from f00f instruction.  Good.");
	tst_exit();
}

int main(void)
{
	signal(SIGILL, sigill);
	tst_resm(TINFO, "Testing for proper f00f instruction handling.");

	/*
	 * an f00f instruction
	 */
	asm volatile (".byte	0xf0\n"
		      ".byte	0x0f\n" ".byte	0xc7\n" ".byte	0xc8\n");

	/*
	 * we shouldn't get here, the f00f instruction should trigger
	 * a SIGILL or lock the system.
	 */
	tst_brkm(TFAIL, NULL,
		 "f00f instruction did not properly cause SIGILL");
}

#else /* __i386__ */

int main(void)
{
	tst_brkm(TCONF, NULL, "f00f bug test only for i386");
}

#endif /* __i386__ */
