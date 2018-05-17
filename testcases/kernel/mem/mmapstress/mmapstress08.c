/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/*
 *   Copyright (c) International Business Machines  Corp., 2003
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

/* as_anon_get:
 *	This program tests the kernel primitive as_anon_get by using up lots of
 *	level 2 page tables causing the kernel to switch to large blocks of
 *	anonymous backing store allocation.  This is done by allocating pages 4
 *	megs apart since each pt handles 1024 pages of 4096 bytes each.  Each
 *	page thus requires another page table.  The pages are then unmapped to
 *	switch back to small swap space allocations.
 */
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
/*****	LTP Port	*****/
#include "test.h"
#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
char *TCID = "mmapstress08";
FILE *temp;
int TST_TOTAL = 1;

#if defined(__i386__) || defined(__x86_64__)
int anyfail();
void ok_exit();
/*****  **      **      *****/

#define NPTEPG		(1024)
/*#define GRAN_NUMBER	(1<<2)*/

#define GRAN_NUMBER	(1<<8)
	/* == 256 @ 4MB per mmap(2), we span a total of 1 GB */

extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern long sysconf(int name);

#define ERROR(M) (void)fprintf(stderr, "%s: errno = %d: " M "\n", argv[0], \
			errno)

 /*ARGSUSED*/ int main(int argc, char *argv[])
{
	caddr_t mmapaddr, munmap_begin;
	long pagesize = sysconf(_SC_PAGE_SIZE);
	int i;
	time_t t;

	(void)time(&t);
	//(void)printf("%s: Started %s", argv[0], ctime(&t));
	if (sbrk(pagesize - ((u_long) sbrk(0) % (u_long) pagesize)) ==
	    (char *)-1) {
		ERROR("couldn't round up brk to a page boundary");
		local_flag = FAILED;
		anyfail();
	}
	/* The brk is now at the begining of a page. */

	if ((munmap_begin = mmapaddr = (caddr_t) sbrk(0)) == (caddr_t) - 1) {
		ERROR("couldn't find top of brk");
		local_flag = FAILED;
		anyfail();
	}
	/* burn level 2 ptes by spacing mmaps 4Meg apart */
	/* This should switch to large anonymous swap space granularity */
	for (i = 0; i < GRAN_NUMBER; i++) {
		if (mmap(mmapaddr, pagesize, PROT_READ | PROT_WRITE,
			 MAP_ANONYMOUS | MAP_PRIVATE, 0, 0) == (caddr_t) - 1) {
			ERROR("mmap failed");
			local_flag = FAILED;
			anyfail();
		}
		mmapaddr += NPTEPG * pagesize;
	}
	/* Free bizillion level2 ptes to switch to small granularity */
	if (munmap(munmap_begin, (size_t) (mmapaddr - munmap_begin))) {
		ERROR("munmap failed");
		local_flag = FAILED;
		anyfail();
	}
	(void)time(&t);
	//(void)printf("%s: Finished %s", argv[0], ctime(&t));
	ok_exit();
	tst_exit();
}

/*****  LTP Port        *****/
void ok_exit(void)
{
	tst_resm(TPASS, "Test passed\n");
	tst_exit();
}

int anyfail(void)
{
	tst_brkm(TFAIL, NULL, "Test failed\n");
}

#else /* defined(__i386__) || defined(__x86_64__) */
int main(void)
{
	tst_brkm(TCONF, NULL, "Test is only applicable for IA-32 and x86-64.");
}
#endif
/*****  **      **      *****/
