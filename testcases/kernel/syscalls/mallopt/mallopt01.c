/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 01/02/2003	Port to LTP	avenkat@us.ibm.com*/
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	mallopt
 *
 * CALLS
 *	malloc(3x), mallopt(3x), mallinfo(3x).
 *
 * ALGORITHM
 *	Set options, malloc memory, and check resource ussage.
 *
 * RESTRICTIONS
 */

#ifdef CONFIG_COLDFIRE
#define __MALLOC_STANDARD__
#endif
#include <errno.h>
/* 
 * NOTE: struct mallinfo is only exported via malloc.h (not stdlib.h), even
 * though it's an obsolete header for malloc(3).
 *
 * Inconsistencies rock.
 */
#include <malloc.h>
#include <stdio.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;

char *TCID = "mallopt01";
int block_number;
FILE *temp;
int TST_TOTAL = 1;
extern int Tst_COUNT;		/* Test Case counter for tst_routines */

void printinfo();

#if !defined(UCLINUX)
struct mallinfo info;

int main(int argc, char *argv[])
{
	char *buf;

	tst_tmpdir();

	buf = SAFE_MALLOC(NULL, 20480);

	/*
	 * Check space usage.
	 */

	info = mallinfo();
	if (info.uordblks < 20480) {
		printinfo();
		tst_resm(TFAIL, "mallinfo failed: uordblks < 20K");
	}
	if (info.smblks != 0) {
		printinfo();
		tst_resm(TFAIL, "mallinfo failed: smblks != 0");
	}
	free(buf);

	/*
	 * Test mallopt's M_MXFAST and M_NLBLKS cmds.
	 */
	mallopt(M_MXFAST, 2048);
	mallopt(M_NLBLKS, 50);
	buf = SAFE_MALLOC(NULL, 1024);

	free(buf);

	unlink("core");
	tst_rmdir();
	tst_exit();
}

void printinfo()
{

	fprintf(stderr, "mallinfo structure:\n");
	fprintf(stderr, "mallinfo.arena = %d\n", info.arena);
	fprintf(stderr, "mallinfo.ordblks = %d\n", info.ordblks);
	fprintf(stderr, "mallinfo.smblks = %d\n", info.smblks);
	fprintf(stderr, "mallinfo.hblkhd = %d\n", info.hblkhd);
	fprintf(stderr, "mallinfo.hblks = %d\n", info.hblks);
	fprintf(stderr, "mallinfo.usmblks = %d\n", info.usmblks);
	fprintf(stderr, "mallinfo.fsmblks = %d\n", info.fsmblks);
	fprintf(stderr, "mallinfo.uordblks = %d\n", info.uordblks);
	fprintf(stderr, "mallinfo.fordblks = %d\n", info.fordblks);
	fprintf(stderr, "mallinfo.keepcost = %d\n", info.keepcost);
}

#else
int main()
{
	tst_brkm(TCONF, NULL, "test is not available on uClinux");
}
#endif /* if !defined(UCLINUX) */
