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

/* 01/02/2003   Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

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
#define _GNU_SOURCE 1

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
	register long long i;
	register int j, k, l,m;

	setup();		/* temp file is now open	*/
/*--------------------------------------------------------------*/
	blenter();

	i = llabs(MININT) + (long long)MININT;
	
	if (i != 0) {
		fprintf(temp, "abs of minimum integer failed.");
		local_flag = FAILED;
	}


	blexit();
/*--------------------------------------------------------------*/
	blenter();

	i = llabs(0);
	if (i != 0) {
		fprintf(temp, "abs(0) failed, returned %lld\n", i);
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	for (m = 1; m >= 0 ; m <<= 1) {
		j = ~m;
		k = j + 1;
		l = abs(k);
		if (l != m)
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
  (local_flag == PASSED ) ? tst_resm(TPASS, "Test passed") : tst_resm(TFAIL, "Test failed");
  return(0);
}

/******			*****/
