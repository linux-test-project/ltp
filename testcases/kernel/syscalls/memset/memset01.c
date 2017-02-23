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

/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	memset1.c -- test setting of  buffer
 *
 * CALLS
 *	memset(3)
 *
 * ALGORITHM
 *	Check boundary conditions, go through 64 byte window.
 *
 * RESTRICTIONS
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "test.h"

char *TCID = "memset01";

#undef BSIZE
#define BSIZE	4096
#define LEN	100
#define FAILED 0
#define PASSED 1

char buf[BSIZE];

int local_flag = PASSED;
int block_number;
int TST_TOTAL = 1;

void fill(void);
int checkit(char *str);

int main(int argc, char *argv[])
{
	register int i, j;
	char *p;

	tst_parse_opts(argc, argv, NULL, NULL);

	local_flag = PASSED;

	fill();

	for (i = 0; i < 200; i++) {
		fill();
		p = &buf[400];
		memset(p, 0, i);
		if ((j = checkit(p)) != i) {
			tst_resm(TINFO,
				 "Not enough zero bytes, wanted %d, got %d", i,
				 j);
			local_flag = FAILED;
			break;
		}
		if (!p[-1] || !p[i]) {
			tst_resm(TINFO, "Boundary error, clear of %d", i);
			local_flag = FAILED;
		}
		if (local_flag == FAILED)
			break;
	}

	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	(local_flag == FAILED) ? tst_resm(TFAIL,
					  "Test failed") : tst_resm(TPASS,
								    "Test passed");
	tst_exit();
}

void fill(void)
{
	register int i;
	for (i = 0; i < BSIZE; i++)
		buf[i] = 'a';
}

int checkit(char *str)
{
	register int i = 0;

	while (!*str++)
		i++;

	return (i);
}
