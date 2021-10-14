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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <stdio.h>		/* needed by testhead.h         */
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <limits.h>

/*****	LTP Port	*****/

#include "test.h"
#define FAILED 0
#define PASSED 1

static const char *TCID = "abs01";
static int local_flag = PASSED;
static int block_number;
static FILE *temp;
static int TST_TOTAL = 1;

static void setup(void);
static int blenter(void);
static int blexit(void);

/********************************/

/*--------------------------------------------------------------*/
int main(void)
{
	register long long i;
	register int j, k, l, m;

	setup();		/* temp file is now open        */
/*--------------------------------------------------------------*/
	blenter();

	i = llabs(INT_MIN) + (long long)INT_MIN;

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

	for (m = 1; m >= 0; m <<= 1) {
		j = ~m;
		k = j + 1;
		l = abs(k);
		if (l != m)
			local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
/* Clean up any files created by test before call to anyfail.	*/

	tst_exit();
}

/*--------------------------------------------------------------*/

/*****  LTP Port	*****/
static void setup(void)
{
	temp = stderr;
}

static int blenter(void)
{
	local_flag = PASSED;
	return (0);
}

static int blexit(void)
{
	(local_flag == PASSED) ? tst_resm(TPASS,
					  "Test passed") : tst_resm(TFAIL,
								    "Test failed");
	return (0);
}

/******			*****/
