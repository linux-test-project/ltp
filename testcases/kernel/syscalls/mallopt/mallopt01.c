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

#include <stdio.h>		/* needed by testhead.h         */
#ifdef CONFIG_COLDFIRE
#define __MALLOC_STANDARD__
#endif
#include <malloc.h>
#include <errno.h>

/*****	LTP Port	*****/
#include "test.h"
#include "usctest.h"

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
/*****	*	*	*****/
struct mallinfo info;

/*--------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	char *buf;
	int flag;

	temp = stderr;
	tst_tmpdir();
/*--------------------------------------------------------------*/
	local_flag = PASSED;

	flag = 0;

	if ((buf = malloc(20480)) == NULL) {
		tst_resm(TBROK, "Reason: Malloc failed! %s", strerror(errno));
		tst_exit();
	}

	/*
	 * Check space usage.
	 */

	info = mallinfo();
	if (info.uordblks < 20480) {
		fprintf(temp, "mallinfo failed: uordblks < 20K\n");
		flag = 1;
		local_flag = FAILED;
	};
	if (info.smblks != 0) {
		fprintf(temp, "mallinfo failed: smblks != 0\n");
		flag = 1;
		local_flag = FAILED;
	}
	if (flag == 1) {
		printinfo();
		flag = 0;
	}
	free(buf);

	/*
	 * Test mallopt's M_MXFAST and M_NLBLKS cmds.
	 */
	mallopt(M_MXFAST, 2048);
	mallopt(M_NLBLKS, 50);
	if ((buf = malloc(1024)) == NULL) {
		tst_resm(TBROK, "Reason:Malloc failed. %s", strerror(errno));
		tst_exit();
	}

	free(buf);

	(local_flag == PASSED) ? tst_resm(TPASS,
					  "Test passed") : tst_resm(TFAIL,
								    "Test failed");
/*--------------------------------------------------------------*/
/* Clean up any files created by test before call to anyfail.	*/

	unlink("core");
	tst_rmdir();
	tst_exit();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
	return 0;
}

/*--------------------------------------------------------------*/
void printinfo()
{

	fprintf(temp, "mallinfo structure:\n");
	fprintf(temp, "mallinfo.arena = %d\n", info.arena);
	fprintf(temp, "mallinfo.ordblks = %d\n", info.ordblks);
	fprintf(temp, "mallinfo.smblks = %d\n", info.smblks);
	fprintf(temp, "mallinfo.hblkhd = %d\n", info.hblkhd);
	fprintf(temp, "mallinfo.hblks = %d\n", info.hblks);
	fprintf(temp, "mallinfo.usmblks = %d\n", info.usmblks);
	fprintf(temp, "mallinfo.fsmblks = %d\n", info.fsmblks);
	fprintf(temp, "mallinfo.uordblks = %d\n", info.uordblks);
	fprintf(temp, "mallinfo.fordblks = %d\n", info.fordblks);
	fprintf(temp, "mallinfo.keepcost = %d\n", info.keepcost);
}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	return 0;
}

#endif /* if !defined(UCLINUX) */
