/* IBM Corporation */
/* 01/02/2003	Port to LTP avenkat@us.ibm.com */
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *      This test mmaps over the tail of the brk segment, growing and
 *	shrinking brk over holes, while changing from small to large and
 *	large to small virtual memory representations.  After mmaping over the
 *	end of the brk segment, it increases the brk which should split
 *	it into two segments (i.e.  |---brk---|-mmap-|--more brk--|).  Next it
 *	decreases the brk segment to the end of the map, and finally decreases
 *	it some more.  Then more vmsegments are created by punching holes in
 *	the brk segments with munmap.  This should cause the vm system to use a
 *	large virtual address space object to keep track of this process.  The
 *	above test is then repeated using the large process object.  After
 *	this, the brk is shrunk to less than 1 page before exiting in order to
 *	test the code which compacts large address space objects.  It also asks
 *	for a huge mmap which is refused.
 */

#define _KMEMUSER
#include <sys/types.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

/*****  LTP Port        *****/
#include "test.h"
#include "usctest.h"
#define FAILED 0
#define PASSED 1

char *TCID = "mmapstress03";
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
void ok_exit();
/*****  **      **      *****/

#define AS_SVSM_VSEG_MAX	48UL
#define AS_SVSM_MMAP_MAX	16UL

#define EXTRA_VSEGS	2L
#define NUM_SEGS	(AS_SVSM_VSEG_MAX + EXTRA_VSEGS)
#define ERROR(M) (void)fprintf(stderr, "%s: errno = %d: " M "\n", progname, \
			errno)
#define NEG1	(char *)-1
#define POINTER_SIZE	(sizeof(void *) << 3)

extern long	sysconf(int name);
extern void	exit(int);
extern time_t	time(time_t *);
extern char	*ctime(const time_t *);
static void	do_test(caddr_t brk_max, long pagesize);

static char *progname;

/*ARGSUSED*/
int
main(int argc, char *argv[])
{
	char *brk_max_addr, *hole_addr, *brk_start, *hole_start;
	size_t pagesize = (size_t)sysconf(_SC_PAGE_SIZE);
	time_t	t;

	progname = argv[0];

	(void)time(&t);
//	(void)printf("%s: Started %s", argv[0], ctime(&t));
	if ((brk_start = sbrk(0)) == NEG1) {
		ERROR("initial sbrk failed");
		anyfail();
	}
	if ((u_long)brk_start % (u_long)pagesize) {
		if (sbrk(pagesize - ((u_long)brk_start % (u_long)pagesize))
			== NEG1)
		{
			ERROR("couldn't round up brk to a page boundary");
        	        anyfail();
		}
	}
	/* The brk is now at the beginning of a page. */

	if ((hole_addr = hole_start = sbrk(NUM_SEGS * 2 * pagesize)) == NEG1) {
		ERROR("couldn't brk large space for segments");
                anyfail();
	}
	if ((brk_max_addr = sbrk(0)) == NEG1) {
		ERROR("couldn't find top of brk");
                anyfail();
	}
	do_test((caddr_t)brk_max_addr, pagesize);

	/* now make holes and repeat test */
	while (hole_addr + pagesize < brk_max_addr) {
		if (munmap(hole_addr, pagesize) == -1) {
			ERROR("failed to munmap odd hole in brk segment");
	                anyfail();
		}
		hole_addr += 2 * pagesize;
	}

	if (brk_max_addr != sbrk(0)) {
		ERROR("do_test should leave the top of brk where it began");
                anyfail();
	}
	do_test((caddr_t)brk_max_addr, pagesize);

	/* Shrink brk */
	if (sbrk(-NUM_SEGS * pagesize) == NEG1) {
		ERROR("couldn't brk back over holes");
                anyfail();
	}
	if ((brk_max_addr = sbrk(0)) == NEG1) {
		ERROR("couldn't find top of break again");
                anyfail();
	}
	/* sbrked over about half the holes */

	hole_addr = hole_start + pagesize; /* munmap the other pages */
	while (hole_addr + pagesize < brk_max_addr) {
		if (munmap(hole_addr, pagesize) == -1) {
			ERROR("failed to munmap even hole in brk segment");
        	        anyfail();
		}
		hole_addr += 2 * pagesize;
	}
	/* munmaped the rest of the brk except a little at the beginning */

	if (brk(brk_start) == -1) {
		ERROR("failed to completely remove brk");
                anyfail();
	}
	if (sbrk(pagesize) == NEG1 || sbrk(-pagesize) == NEG1) {
		ERROR("failed to fiddle with brk at the end");
                anyfail();
	}
	/* Ask for a ridiculously large mmap region at a high address */
	if (mmap((caddr_t)(1UL << (POINTER_SIZE  - 1)) - pagesize,
		(size_t)((1UL << (POINTER_SIZE - 1)) - pagesize),
		PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_FIXED|MAP_SHARED, 0, 0)
		!= (caddr_t)-1)
	{
		ERROR("really large mmap didn't fail");
                anyfail();
	}
	if (errno != ENOMEM && errno != EINVAL) {
		ERROR("really large mmap didn't set errno = ENOMEM nor EINVAL");
                anyfail();
	}
	(void)time(&t);
//	(void)printf("%s: Finished %s", argv[0], ctime(&t));
	ok_exit();
	tst_exit();
}

/*
 * do_test assumes that brk_max is a multiple of pagesize
 */

static
void
do_test(caddr_t brk_max, long pagesize)
{
	if (mmap((caddr_t)((long)brk_max - 3*pagesize), (2*pagesize),
		PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE, 0, 0)
		== (caddr_t)-1)
	{
		ERROR("mmap failed");
		anyfail();
	}
	/* extend mmap */
	if (mmap((caddr_t)((long)brk_max - 2*pagesize), (2*pagesize),
		PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE, 0, 0)
		== (caddr_t)-1)
	{
		ERROR("mmap failed");
		anyfail();
	}
	if (sbrk(pagesize) == NEG1) {
		ERROR("sbrk failed to grow over mmaped region");
		anyfail();
	}
	if (sbrk(-pagesize) == NEG1) {
		ERROR("sbrk failed to shrink back to mmaped region");
		anyfail();
	}
	if (sbrk(-pagesize) == NEG1) {
		ERROR("sbrk failed to shrink over mmaped region more");
		anyfail();
	}
	if (sbrk(-pagesize) == NEG1) {
		ERROR("sbrk failed to shrink some more");
		anyfail();
	}
	if (sbrk(2 * pagesize) == NEG1) {
		ERROR("sbrk failed to change brk segment to original size");
		anyfail();
	}
}

/*****  LTP Port        *****/
void ok_exit()
{
        tst_resm(TPASS, "Test passed");
        tst_exit();
}

int anyfail()
{
  tst_resm(TFAIL, "Test failed");
  tst_exit();
        return 0;
}

/*****  **      **      *****/