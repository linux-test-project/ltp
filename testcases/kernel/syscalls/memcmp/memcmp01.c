/* IBM Corporation */
/* 01/02/2003	Port to LTP	avenkat&us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * $Copyright:	$
 * Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989 
 * Sequent Computer Systems, Inc.   All rights reserved.
 *  
 * This software is furnished under a license and may be used
 * only in accordance with the terms of that license and with the
 * inclusion of the above copyright notice.   This software may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.
 */

/*
 * NAME
 *	memcmp1 -- buffer  compare
 *
 * CALLS
 *	memcmp(3)
 *
 * ALGORITHM
 *	Check boundary conditions.
 *
 * RESTRICTIONS
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/*****	LTP Port	*****/
#include "test.h"
#include "usctest.h"


char *TCID = "memcmp1";

#undef  BSIZE
#define BSIZE	4096
#define LEN	100
#define FAILED 0
#define PASSED 1
/*****	**	**	*****/

char buf[BSIZE];

/*****	LTP Port	*****/
int local_flag = PASSED;
int block_number;
int errno;
FILE *temp;
int TST_TOTAL =  2;
int anyfail();
int blenter();
int blexit();
int instress();
extern int Tst_count;

void setup();
/*****	**	**	*****/

void clearit();
void fill(char *str);
int checkit(char *str);

/*--------------------------------------------------------------*/
int main (int argc, char *argv[])
{
	char *p, *q;

	setup();		/* temp file is now open	*/
/*--------------------------------------------------------------*/
	blenter();

	clearit();

	p = &buf[100];
	q = &buf[800];

	fill(p);
	fill(q);

	if (memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp fails - should have succeeded.\n");
		local_flag = FAILED;
	}

	p[LEN - 1] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	p[LEN -1] = 'a';
	p[0] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	p[0] = 'a';
	q[LEN-1] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	q[LEN-1] = 'a';
	q[0] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	q[0] = 'a';

	if (memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp fails - should have succeeded.\n");
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	clearit();

	p = &buf[800];
	q = &buf[100];

	fill(p);
	fill(q);

	if (memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp fails - should have succeeded.\n");
		local_flag = FAILED;
	}

	p[LEN - 1] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	p[LEN -1] = 'a';
	p[0] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	p[0] = 'a';
	q[LEN-1] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	q[LEN-1] = 'a';
	q[0] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	q[0] = 'a';

	if (memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp fails - should have succeeded.\n");
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
/* Clean up any files created by test before call to anyfail.	*/

	anyfail();	/* THIS CALL DOES NOT RETURN - EXITS!!	*/
	return(0);
}
/*--------------------------------------------------------------*/
/* FUNCTIONS GO HERE */

void clearit()
{
	register int i;

	for (i=0; i < BSIZE; i++)
		buf[i] = 0;
}

void fill(char *str)
{
	register int i;
	for (i=0; i < LEN; i++)
		*str++ = 'a';
}

int checkit(char *str)
{
	register int i;
	for (i=0; i < LEN; i++)
		if (*str++ != 'a')
			return(-1);
	
	return (0);
}


int anyfail()
{
  tst_exit();
  return(0);
}

void setup()
{
 temp = stderr;
}

int blenter()
{
   local_flag = PASSED;
   return(0);
}

int blexit()
{
   (local_flag == FAILED) ? tst_resm(TFAIL, "Test failed\n") : tst_resm(TPASS, "Test passed\n");
   return(0);
}


