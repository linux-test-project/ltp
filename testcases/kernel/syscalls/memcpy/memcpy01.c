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

/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	memcpy  --  test memcpy
 *
 * CALLS
 *	memcpy1(3)
 *
 * ALGORITHM
 *	There are 4 cases for copies:  S = Source, D = Destination
 *
 *	  1 - S < D no overlap
 *	  2 - D < S no overlap
 *	  3 - S < D with overlap
 *	  4 - D < S with overlap
 *
 *	We try all four cases.  Check buffer boundaries.
 *
 * RESTRICTIONS
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/*****	LTP Port	*****/
#include "test.h"
#include "usctest.h"

char *TCID = "memcpy1";
/*****	**	**	*****/
#undef  BSIZE
#define BSIZE	4096
#define LEN	100
#define FAILED 0
#define PASSED 1

/*****	LTP Port	*****/
int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
/*****	**	**	*****/
char buf[BSIZE];

/*****	LTP Port	*****/
extern int Tst_count;

int anyfail();
int blenter();
int blexit();

void setup();
/*****	**	**	*****/
void clearit();
void fill(char *str);
int checkit(char *str);

/*--------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	char *p, *q;

	setup();		/* temp file is now open        */
/*--------------------------------------------------------------*/
	blenter();

	clearit();

	p = &buf[100];

	fill(p);
	q = &buf[800];
	memcpy(q, p, LEN);

	if (checkit(q)) {
		fprintf(temp, "\tcopy failed - missed data\n");
		local_flag = FAILED;
	}

	if (p[-1] || p[LEN]) {
		fprintf(temp, "\tcopy failed - 'to' bounds\n");
		local_flag = FAILED;
	}

	if (q[-1] || q[LEN]) {
		fprintf(temp, "\tcopy failed - 'from' bounds\n");
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	clearit();

	p = &buf[800];

	fill(p);
	q = &buf[100];
	memcpy(q, p, LEN);

	if (checkit(q)) {
		fprintf(temp, "\tcopy failed - missed data\n");
		local_flag = FAILED;
	}

	if (p[-1] || p[LEN]) {
		fprintf(temp, "\tcopy failed - 'to' bounds\n");
		local_flag = FAILED;
	}

	if (q[-1] || q[LEN]) {
		fprintf(temp, "\tcopy failed - 'from' bounds\n");
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	clearit();

	p = &buf[800];

	fill(p);
	q = &buf[850];
	memcpy(q, p, LEN);

	if (checkit(q)) {
		fprintf(temp, "\tcopy failed - missed data\n");
		local_flag = FAILED;
	}

	if (p[-1]) {
		fprintf(temp, "\tcopy failed - 'to' bounds\n");
		local_flag = FAILED;
	}

	if (q[LEN]) {
		fprintf(temp, "\tcopy failed - 'from' bounds\n");
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	clearit();

	p = &buf[850];

	fill(p);
	q = &buf[800];
	memcpy(q, p, LEN);

	if (checkit(q)) {
		fprintf(temp, "\tcopy failed - missed data\n");
		local_flag = FAILED;
	}

	if (p[LEN]) {
		fprintf(temp, "\tcopy failed - 'to' bounds\n");
		local_flag = FAILED;
	}

	if (q[-1]) {
		fprintf(temp, "\tcopy failed - 'from' bounds\n");
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
/* Clean up any files created by test before call to anyfail.	*/

	anyfail();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
	return 0;
}

/*--------------------------------------------------------------*/
/* FUNCTIONS GO HERE */

void clearit()
{
	register int i;

	for (i = 0; i < BSIZE; i++)
		buf[i] = 0;
}

void fill(char *str)
{
	register int i;
	for (i = 0; i < LEN; i++)
		*str++ = 'a';
}

int checkit(char *str)
{
	register int i;
	for (i = 0; i < LEN; i++)
		if (*str++ != 'a')
			return (-1);

	return (0);
}

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
	local_flag = PASSED;
	return 0;
}

int blexit()
{
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	return 0;
}
