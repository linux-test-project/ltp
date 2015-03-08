/* IBM Corporation */
/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/*
 *   Copyright (c) International Business Machines  Corp., 2003
 *
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

/*
 * mfile_swap:
 * Mmap a large (> ANON_GRAN_PAGES_MAX) shared, anonymous region before
 * swapping to use the second half of the kernel primitive mfile_swap.
 * However, this test does _NOT_ cause swapping to occur.  Instead it should be
 * run with waves or at the same time as a test which does cause swapping (i.e.
 * vsswapin or vfork_swap)
 */
#define _KMEMUSER
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/*****  LTP Port        *****/
#include "test.h"
#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
char *TCID = "mmapstress06";	//mfile_swap
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
void ok_exit();
/*****	**	**	*****/

#define ANON_GRAN_PAGES_MAX	(32U)

extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern int atoi(const char *);

#define NMFPTEPG		(1024)
#define ERROR(M)	(void)fprintf(stderr, "%s: errno = %d; " M "\n", \
				argv[0], errno);

int main(int argc, char *argv[])
{
	caddr_t mmapaddr;
	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	time_t t;
	int sleep_time = 0;

	if (!argc) {
		(void)fprintf(stderr, "argc == 0\n");
		anyfail();
	}
	if (argc != 2 || !(sleep_time = atoi(argv[1]))) {
		(void)fprintf(stderr, "usage: %s sleep_time\n", argv[0]);
		anyfail();
	}
	(void)time(&t);
//      (void)printf("%s: Started %s", argv[0], ctime(&t));  LTP Port
	if (sbrk(pagesize - ((ulong) sbrk(0) & (pagesize - 1))) == (char *)-1) {
		ERROR("couldn't round up brk");
		anyfail();
	}
	if ((mmapaddr = sbrk(0)) == (char *)-1) {
		ERROR("couldn't find top of brk");
		anyfail();
	}
	/* mmapaddr is now on a page boundary after the brk segment */
	if (mmap
	    (mmapaddr, (ANON_GRAN_PAGES_MAX * NMFPTEPG + 1) * pagesize,
	     PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0,
	     0) == (caddr_t) - 1) {
		ERROR("large mmap failed");
		printf("for this test to run, it needs a mmap space of\n");
		printf("%d pages\n", (ANON_GRAN_PAGES_MAX * NMFPTEPG + 1));
		return 1;
	}
	(void)sleep(sleep_time);
	(void)time(&t);
//      (void)printf("%s: Finished %s", argv[0], ctime(&t)); LTP Port
	ok_exit();
	tst_exit();
}

/*****	LTP Port	*****/
void ok_exit(void)
{
	tst_resm(TPASS, "Test passed\n");
	tst_exit();
}

int anyfail(void)
{
	tst_brkm(TFAIL, NULL, "Test failed\n");
}

/*****	**	**	*****/
