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

/* 01/02/2003	Port to LTP	avenkat&us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

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

#include "test.h"

char *TCID = "memcmp1";

#undef  BSIZE
#define BSIZE	4096
#define LEN	100
#define FAILED 0
#define PASSED 1

char buf[BSIZE];

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 2;
int anyfail();
int blenter();
int blexit();
int instress();

void setup();

void clearit();
void fill(char *str);
int checkit(char *str);

int main(int argc, char *argv[])
{
	char *p, *q;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();
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

	p[LEN - 1] = 'a';
	p[0] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	p[0] = 'a';
	q[LEN - 1] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	q[LEN - 1] = 'a';
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

	p[LEN - 1] = 'a';
	p[0] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	p[0] = 'a';
	q[LEN - 1] = 0;

	if (!memcmp(p, q, LEN)) {
		fprintf(temp, "\tmemcmp succeeded - should have failed.\n");
		local_flag = FAILED;
	};

	q[LEN - 1] = 'a';
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

	anyfail();
	tst_exit();
}

void clearit(void)
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

int anyfail(void)
{
	tst_exit();
}

void setup(void)
{
	temp = stderr;
}

int blenter(void)
{
	local_flag = PASSED;
	return 0;
}

int blexit(void)
{
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	return 0;
}
