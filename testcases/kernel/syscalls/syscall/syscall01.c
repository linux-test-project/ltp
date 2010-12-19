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

/* 01/02/2003	Port to LTP avenkat@us.ibm.com*/
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	syscall1.c -- test syscall
 *
 * CALLS
 *	syscall
 *
 * ALGORITHM
 *	Use syscall to simulate some section 2 calls and make sure
 *	things work as expected.  Pretty simple, but if it works
 *	for a few it should work for all.
 *
 * RESTRICTIONS
 *	The syscall numbers are system dependent!!  They represent
 *	entries in a table and can be changed from kernel to kernel.
 *	They ARE differnet between vax 4.2BSD and our ported system.
 */

#include <stdio.h>		/* needed by testhead.h         */
#include <syscall.h>
#include <errno.h>

/*****	LTP Port	*****/
#include "test.h"
#include "usctest.h"

#define FAILED 0
#define PASSED 1

char *TCID = "syscall01";
int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
/*****	**	**	*****/

//char progname[]= "syscall1()";

#define ITER		500

int t_flag;

/*****	LTP Port	*****/
void setup();
int blenter();
int blexit();
int anyfail();
void cleanup();
void do_setpg();
void fail_exit();
/*****	**	**	*****/

/*--------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	register int i;
	int v1, v2;

	setup();		/* temp file is now open        */
/*--------------------------------------------------------------*/
	blenter();

#if defined(SYS_getpid)
	for (i = 0; i < ITER; i++) {
		v1 = getpid();
		v2 = syscall(SYS_getpid);
		if (v1 != v2) {
			fprintf(temp, "\tgetpid syscall failed.\n");
			fprintf(temp, "\t  iteration %d\n", i);
			local_flag = FAILED;
			break;
		}
	}
#else
	fprintf(temp, "\tgetpid syscall failed.\n");
	fprintf(temp, "\tSYS_getpid not defined\n");
	local_flag = FAILED;
#endif
	blexit();
/*--------------------------------------------------------------*/
	blenter();

#if defined(SYS_getuid) || defined(SYS_getuid32)
	for (i = 0; i < ITER; i++) {
		v1 = getuid();
#if defined(SYS_getuid)
		v2 = syscall(SYS_getuid);
#else
		v2 = syscall(SYS_getuid32);
#endif
		if (v1 != v2) {
			fprintf(temp, "\tgetuid syscall failed.\n");
			fprintf(temp, "\t  iteration %d\n", i);
			local_flag = FAILED;
			break;
		}
	}
#else
	fprintf(temp, "\tgetuid syscall failed.\n");
	fprintf(temp, "\tSYS_getuid and SYS_getuid32 not defined\n");
	local_flag = FAILED;
#endif
	blexit();
/*--------------------------------------------------------------*/
	blenter();

#if defined(SYS_getgid) || defined(SYS_getgid32)
	for (i = 0; i < ITER; i++) {
		v1 = getgid();
#if defined(SYS_getgid)
		v2 = syscall(SYS_getgid);
#else
		v2 = syscall(SYS_getgid32);
#endif
		if (v1 != v2) {
			fprintf(temp, "\tgetgid syscall failed.\n");
			fprintf(temp, "\t  iteration %d\n", i);
			local_flag = FAILED;
			break;
		}
	}
#else
	fprintf(temp, "\tgetgid syscall failed.\n");
	fprintf(temp, "\tSYS_getgid and SYS_getgid32 not defined\n");
	local_flag = FAILED;
#endif

	blexit();
/*--------------------------------------------------------------*/

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();
	tst_exit();

	anyfail();		/* THIS CALL DOES NOT RETURN - EXITS!!  */

}

/*--------------------------------------------------------------*/

/*****	LTP Port	*****/
/* functions */

void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}

void setup()
{

	temp = stderr;

}

int blenter()
{
	local_flag = PASSED;
	return 0;
}

int blexit()
{
	(local_flag == PASSED) ? tst_resm(TPASS, "Test passed")
	    : tst_resm(TFAIL, "Test failed");
	return 0;
}

int anyfail()
{
	tst_exit();
	return 0;
}

void fail_exit()
{
	local_flag = FAILED;
	blexit();
	anyfail();
}

/*****	**	**	*****/