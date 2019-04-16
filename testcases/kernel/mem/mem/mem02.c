/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *   Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
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

/* IBM Corporation */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 10/30/2002	Port to LTP	dbarrera@us.ibm.com */

/*======================================================================
/	=================== TESTPLAN SEGMENT ===================
>KEYS:	< calloc, malloc, free, realloc, valloc
>WHAT:	< check that memory can be allocated and freed. check for zeroed
   	< memory.
>HOW:	< Allocate a big chunk of memory, verify it is available (zeroed
	< in the case of calloc).
	< Write into it and verify, free memory and verify free was
	< successful.
	< In the case of valloc, allocate memory and free it (do this for
	< several iterations). Check if valloc returns unaligned pointers.
	< If valloc causes a SIGSEGV, that means a failure has occured.
>BUGS:	<
======================================================================*/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "test.h"
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/user.h>

#define MEMSIZE	8192 * 8192

void on_mem_fault(int sig);

char *TCID = "mem02";
int TST_TOTAL = 1;

static void usage(char *progname)
{
	fprintf(stderr, "usage: %s -m memsize\n", progname);
	fprintf(stderr, "\t-m specify the size of memory to allocate, in MB\n");
	exit(1);
}

static int get_pagesize_order(void)
{
	int i, pagesize, size = 0;
	pagesize = sysconf(_SC_PAGESIZE);
	for (i = 0; size != pagesize; i++)
		size = 1 << i;
	return (i - 1);
}

int main(int argc, char **argv)
{
	int i;
	char *pm1, *pm2, *pm3, *pm4;
	void *memptr;
	long laddr;
	int iteration_count;
	int size;		/* Size to memory to be valloced */
	int pagesize_order = get_pagesize_order();
	int memsize = MEMSIZE;	/* Size of memory to allocate */
	extern char *optarg;	/* getopt() function global variables */
	extern int optopt;	/* stores bad option passed to the program */
	int ch;

	optarg = NULL;
	opterr = 0;

	while ((ch = getopt(argc, argv, "m:")) != -1) {
		switch (ch) {
		case 'm':
			if (optarg)
				memsize = atoi(optarg) * 1024 * 1024;
			else
				fprintf(stderr, "%s: option -%c requires "
					"an argument\n", argv[0], optopt);
			break;
		default:
			usage(argv[0]);
			exit(1);
		}
	}

	/* check out calloc/free */
	if ((pm2 = pm1 = calloc(memsize, 1)) == NULL) {

		tst_brkm(TFAIL, NULL, "calloc - alloc of %dMB failed",
			 memsize / 1024 / 1024);
	}

	for (i = 0; i < memsize; i++)
		if (*pm2++ != 0) {
			tst_brkm(TFAIL, NULL,
				 "calloc returned non zero memory");
		}

	pm2 = pm1;
	for (i = 0; i < memsize; i++)
		*pm2++ = 'X';
	pm2 = pm1;
	for (i = 0; i < memsize; i++)
		if (*pm2++ != 'X') {
			tst_brkm(TFAIL, NULL,
				 "could not write/verify memory ");
		}

	free(pm1);

	tst_resm(TPASS, "calloc - calloc of %uMB of memory succeeded",
		 memsize / 1024 / 1024);

/*--------------------------------------------------------------------*/

	/* check out malloc/free */
	if ((pm2 = pm1 = malloc(memsize)) == NULL) {
		tst_brkm(TFAIL, NULL, "malloc did not alloc memory ");
	}

	for (i = 0; i < memsize; i++)
		*pm2++ = 'X';
	pm2 = pm1;
	for (i = 0; i < memsize; i++)
		if (*pm2++ != 'X') {
			tst_brkm(TFAIL, NULL,
				 "could not write/verify memory ");
		}

	free(pm1);

	tst_resm(TPASS, "malloc - malloc of %uMB of memory succeeded",
		 memsize / 1024 / 1024);

/*--------------------------------------------------------------------*/

	/* check out realloc */

	pm4 = pm3 = malloc(10);
	for (i = 0; i < 10; i++)
		*pm4++ = 'X';

	/* realloc with reduced size */
	pm4 = realloc(pm3, 5);
	pm3 = pm4;
	/* verify contents did not change */
	for (i = 0; i < 5; i++) {
		if (*pm4++ != 'X') {
			tst_brkm(TFAIL, NULL,
				 "realloc changed memory contents");
		}
	}

	tst_resm(TPASS, "realloc - realloc of 5 bytes succeeded");

	/* realloc with increased size after fragmenting memory */
	pm4 = realloc(pm3, 15);
	pm3 = pm4;
	/* verify contents did not change */
	for (i = 0; i < 5; i++) {
		if (*pm3++ != 'X') {
			tst_brkm(TFAIL, NULL,
				 "realloc changed memory contents");
		}
	}

	tst_resm(TPASS, "realloc - realloc of 15 bytes succeeded");
	free(pm4);

/*--------------------------------------------------------------------*/
	/*
	 * Check out for valloc failures
	 */

	/*
	 * Setup to catch the memory fault, otherwise the core might
	 * be dumped on failures.
	 */
	if ((signal(SIGSEGV, on_mem_fault)) == SIG_ERR) {
		tst_brkm(TFAIL, NULL,
			 "Could not get signal handler for SIGSEGV");
	}

	srand(1);		/* Ensure Determinism         */

	for (iteration_count = 15000; iteration_count > 0; iteration_count--) {
		/*
		 * size is a fraction of 100000 and is determined by rand().
		 */
		size = (int)((rand() / (float)RAND_MAX) * 100000) + 1;
		memptr = valloc(size);

		/*
		 * Check to see if valloc returns unaligned data.
		 * This can be done by copying the memory address into
		 * a variable and the by diving and multipying the address
		 * by the pagesize order and checking.
		 */
		laddr = (long)memptr;
		if (((laddr >> pagesize_order) << pagesize_order) != laddr) {
			tst_brkm(TFAIL, NULL,
				 "Valloc returned unaligned data");
		}

		free(memptr);
	}
	tst_resm(TPASS, "valloc - valloc of rand() size of memory succeeded "
		 "for 15000 iteration");

	tst_exit();
}

/*
 * void
 * on_mem_fault(int sig)
 *
 *	on_mem_fault() is a signal handler used by the valloc test-case
 *	(block 3). This function will catch the signal, indicate a failure,
 *	write to the log file (a failure message) and exit the test.
 */
void on_mem_fault(int sig)
{
	tst_brkm(TFAIL, NULL, "\tTest failed on receipt of a SIGSEGV signal");
}
