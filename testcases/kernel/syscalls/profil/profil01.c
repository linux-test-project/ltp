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

/* 01/02/2003	Port to LTP avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	profil1.c  -- test profil procedure
 *
 * CALLS
 *	profil(2), alarm(2), signal(2)
 *
 * ALGORITHM
 *	Set up a profiling buffer, turn profiling on, set a timer for
 *	cpu time, spin the pc and wait for timer to go off.
 *	The profiling buffer should contain some info, highly concentrated.
 *	We just do a "looks reasonable" check.
 *
 * RESTRICTIONS
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <features.h>
#ifndef __UCLIBC__

#ifdef __arm__
#define ADDRESS_OFFSET 0x8000
#else
#define ADDRESS_OFFSET 0
#endif

#include <stdio.h>
#include <signal.h>
#include <limits.h>
/*****	LTP Port	*****/
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"
#define FAILED 0
#define PASSED 1
/*****	**	**	*****/

#define P_TIME		10	/* profile for this many  seconds */

extern int etext;

volatile int t_flag;

//char progname[]= "profil1()";

/*****	LTP Port	*****/
char *TCID = "profil01";
int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
struct sigaction sigptr;

int anyfail();
int blenter();
int blexit();
void setup();
void terror();
void fail_exit();
/*****	**	**	*****/

u_short *pbuf;
int stuff[11];
int loops_completed;
int ucount, scount;

/*--------------------------------------------------------------*/
int main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	int count, loc;
	long int bsize;
	void alrm();
#ifdef __mips__
#if _MIPS_SIM == _MIPS_SIM_ABI64
	extern long int __start;
	long int lotext = (long int)&__start;
#else
	extern int __start;
	int lotext = (int)&__start;
#endif
#elif defined(__powerpc64__)
	extern long int _start;
	long int *lotextptr = (long *)&_start;
	long int lotext = *lotextptr;
#elif __WORDSIZE == 64
	extern long int _start;
	long int lotext = (long)&_start;
#else
	extern int _start;
	int lotext = (int)&_start;
#endif

	bsize = (long int)&etext;
	bsize -= lotext & ~4096;

	count = loc = 0;

	setup();		/* temp file is now open        */
	/*
	   if ((sigset(SIGALRM, alrm)) == SIG_ERR) {
	   fprintf(temp,"signal failed. errno = %d\n",errno);
	   fail_exit();
	   } */
	sigptr.sa_handler = (void (*)(int signal))alrm;
	sigfillset(&sigptr.sa_mask);
	sigptr.sa_flags = 0;
	sigaddset(&sigptr.sa_mask, SIGALRM);
	if (sigaction(SIGALRM, &sigptr, NULL) == -1) {
		fprintf(temp, "Signal SIGALRM failed, errno = %d \n", errno);
		fail_exit();
	}

/*--------------------------------------------------------------*/
	blenter();

	if ((pbuf =
	     (u_short *) malloc(bsize * (sizeof(u_short)))) == (u_short *) 0) {
		fprintf(temp, "\tcannot malloc buffer.\n");
		fail_exit();
	}

	for (i = 0; i < bsize; i++)
		pbuf[i] = 0;

	if (profil(pbuf, bsize, ADDRESS_OFFSET, 0x10000)) {
		fprintf(temp, "\tprofile (on) failed, errno = %d\n", errno);
		fail_exit();
	}

	/*
	 * Set timer.
	 * Code will just loop in small area of text.
	 */

	alarm(P_TIME);

	while (!t_flag) {
		stuff[0] = 1;
		stuff[1] = 1;
		stuff[2] = 1;
		stuff[3] = 1;
		stuff[4] = 1;
		stuff[5] = 1;
		stuff[6] = 1;
		stuff[7] = 1;
		stuff[8] = 1;
		stuff[9] = 1;
		stuff[10] = 1;
		loops_completed++;
	}

	if (profil(pbuf, bsize, ADDRESS_OFFSET, 0)) {
		fprintf(temp, "\tprofile (off) failed, errno = %d\n", errno);
		fail_exit();
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	for (i = 0; i < bsize; i++) {
		count += pbuf[i];
		if (pbuf[i])
			loc++;
	}
	ucount = count;

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	if ((sigset(SIGCLD, SIG_IGN)) == SIG_ERR) {
		fprintf(temp, "signal failed. errno = %d\n", errno);
		fail_exit();
	}
	t_flag = 0;
	setpgrp();
	for (i = 0; i < bsize; i++)
		pbuf[i] = 0;

	if (profil(pbuf, bsize, ADDRESS_OFFSET, 0x10000)) {
		fprintf(temp, "\tprofile (on) failed, errno = %d\n", errno);
		fail_exit();
	}

	/*
	 * Set timer. This loop will spend a lot of time in system code
	 * (searching though proc table) that won't add to our buffer
	 * since the pc isn't in our code.
	 */

	alarm(P_TIME);

	while (!t_flag) {
		kill(getpid(), SIGCLD);
	}

	if (profil(pbuf, bsize, ADDRESS_OFFSET, 0)) {
		fprintf(temp, "\tprofile (off) failed, errno = %d\n", errno);
		fail_exit();
	}

	count = 0;
	for (i = 0; i < bsize; i++) {
		count += pbuf[i];
		if (pbuf[i])
			loc++;
	}
	scount = count;

	if (scount > ucount) {
		fprintf(temp, "\tUnexpected profiling results.\n");
		fprintf(temp, "\tExpected second value to be less.\n");
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	anyfail();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
	tst_exit();
}

/*--------------------------------------------------------------*/

void alrm()
{
	t_flag++;
}

/*****	LTP Port	*****/
int anyfail()
{
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	tst_exit();
	return 0;
}

void setup()
{
	temp = stderr;
}

int blenter()
{
	//tst_resm(TINFO, "Enter block %d", block_number);
	local_flag = PASSED;
	return 0;
}

int blexit()
{
	//tst_resm(TINFO, "Exitng test");
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	return 0;
}

void terror(char *message)
{
	tst_resm(TBROK, "Reason: %s:%s", message, strerror(errno));
}

void fail_exit()
{
	local_flag = FAILED;
	anyfail();

}

/*****	**	**	*****/

#else
int main(void)
{
	/* uClibc does not have profiling support */
	tst_exit();
}
#endif