/* IBM Corporation */
/* 01/02/2003   Port to LTP	avenkat@us.ibm.com */
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
 *	abs -- absolute integer value
 *
 * CALLS
 *	abs(3)
 *
 * ALGORITHM
 *	Check with variables.  Also most neg value as listed
 *	on man page.
 *
 * RESTRICTIONS
 *	considered a long time - estimate this one
 */

#include <stdio.h>		/* needed by testhead.h		*/
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <values.h>

/*****	LTP Port	*****/

#include "test.h"
#include "usctest.h"
#define FAILED 0
#define PASSED 1

char *TCID = "abs01";
int local_flag = PASSED;
int block_number;
int errno;
FILE * temp;
int TST_TOTAL =1;
extern int Tst_count;


void setup();
int blenter();
int blexit();

/********************************/

/*--------------------------------------------------------------*/
int main (argc, argv)
	int  argc;
	char *argv[];
{
	register int i,j, k, l;

	setup();		/* temp file is now open	*/
/*--------------------------------------------------------------*/
	blenter();

	i = abs(MININT);
	if (i != MININT) {
		fprintf(temp, "abs of minimum integer failed.");
		local_flag = FAILED;
	}


	blexit();
/*--------------------------------------------------------------*/
	blenter();

	i = abs(0);
	if (i != 0) {
		fprintf(temp, "abs(0) failed, returned 0x%x\n", i);
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	for (i = 1; i >= 0 ; i <<= 1) {
		j = ~i;
		k = j + 1;
		l = abs(k);
		if (l != i)
			local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
/* Clean up any files created by test before call to anyfail.	*/

	tst_exit();	/* THIS CALL DOES NOT RETURN - EXITS!!	*/
	return(0);
}
/*--------------------------------------------------------------*/

/*****  LTP Port	*****/
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
  (local_flag == PASSED ) ? tst_resm(TPASS, "Test passed\n") : tst_resm(TFAIL, "Test failed\n");
  return(0);
}

/******			*****/
