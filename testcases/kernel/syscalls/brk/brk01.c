/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: brk01.c,v 1.8 2009/08/28 11:24:25 vapier Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: brk01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for brk(2)
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 1
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: William Roske
 *
 *    CO-PILOT		: Dave Fenner
 *
 *    DATE STARTED	: 03/30/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) brk(2) returns...(See Description)
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *$
 *    ENVIRONMENTAL NEEDS
 * 	The libcuts.a and libsys.a libraries must be included in
 *	the compilation of this test.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the brk(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	brk(2).
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/resource.h>

#include "test.h"
#include "usctest.h"

#ifndef BSIZE
#define BSIZE  BBSIZE
#endif

void setup();
void cleanup();

#define MAX_SIZE_LC	1000	/* loop count test will reach max size */

char *TCID = "brk01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

long Max_brk_byte_size;
long Beg_brk_val;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int incr;		/* increment */
	long nbrkpt;		/* new brk point value */
	long cur_brk_val;	/* current size returned by sbrk */
	long aft_brk_val;	/* current size returned by sbrk */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/*
	 * Attempt to control how fast we get to test max size.
	 * Every MAX_SIZE_LC'th lc will be fastest test will reach max size.
	 */
	incr = (Max_brk_byte_size - Beg_brk_val) / (MAX_SIZE_LC / 2);

	if ((incr * 2) < 4096)	/* make sure that process will grow */
		incr += 4096 / 2;

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Determine new value to give brk
		 * Every even lc value, grow by 2 incr and
		 * every odd lc value, strink by one incr.
		 * If lc is equal to 3, no change, special case.
		 */
		cur_brk_val = (long)sbrk(0);
		if (lc == 3) {
			nbrkpt = cur_brk_val;	/* no change, special one time case */
		} else if ((lc % 2) == 0) {
			/*
			 * grow
			 */
			nbrkpt = cur_brk_val + (2 * incr);

			if (nbrkpt > Max_brk_byte_size)
				nbrkpt = Beg_brk_val;	/* start over */

		} else {
			/*
			 * shrink
			 */
			nbrkpt = cur_brk_val - incr;
		}

/****
    printf("cur_brk_val = %d, nbrkpt = %d, incr = %d, lc = %d\n",
	cur_brk_val, nbrkpt, incr, lc);
****/

		/*
		 * Call brk(2)
		 */
		TEST(brk((char *)nbrkpt));

		/* check return code */
		if (TEST_RETURN == -1) {

			aft_brk_val = (long)sbrk(0);
			tst_resm(TFAIL|TTERRNO,
				 "brk(%ld) failed (size before %ld, after %ld)",
				 nbrkpt, cur_brk_val, aft_brk_val);

		} else {

			if (STD_FUNCTIONAL_TEST) {

				aft_brk_val = (long)sbrk(0);
				if (aft_brk_val == nbrkpt) {

					tst_resm(TPASS,
						 "brk(%ld) returned %ld, new size verified by sbrk",
						 nbrkpt, TEST_RETURN);
				} else {
					tst_resm(TFAIL,
						 "brk(%ld) returned %ld, sbrk before %ld, after %ld",
						 nbrkpt, TEST_RETURN,
						 cur_brk_val, aft_brk_val);
				}
			}
		}

	}

	cleanup();

	tst_exit();
}

void setup()
{
	unsigned long max_size;
	int ncpus;
	unsigned long ulim_sz;
	unsigned long usr_mem_sz;
	struct rlimit lim;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/*if ((ulim_sz=ulimit(3,0)) == -1)
	   tst_brkm(TBROK|TERRNO, cleanup, "ulimit(3,0) failed"); */

	if (getrlimit(RLIMIT_DATA, &lim) == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "getrlimit(RLIMIT_DATA,%p) failed", &lim);
	ulim_sz = lim.rlim_cur;

#ifdef CRAY
	if ((usr_mem_sz = sysconf(_SC_CRAY_USRMEM)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "sysconf(_SC_CRAY_USRMEM) failed");

	usr_mem_sz *= 8;	/* convert to bytes */
#else
	/*
	 * On IRIX, which is a demand paged system, memory is managed
	 * different than on Crays systems.  For now, pick some value.
	 */
	usr_mem_sz = 1024 * 1024 * sizeof(long);
#endif

#ifdef __linux__
#define _SC_NPROC_ONLN _SC_NPROCESSORS_ONLN
#endif
	if ((ncpus = sysconf(_SC_NPROC_ONLN)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "sysconf(_SC_NPROC_ONLN) failed");

	/*
	 * allow 2*ncpus copies to run.
	 * never attempt to take more than a * 1/4 of memory (by single test)
	 */

	if (ulim_sz < usr_mem_sz)
		max_size = ulim_sz;
	else
		max_size = usr_mem_sz;

	max_size = max_size / (2 * ncpus);

	if (max_size > (usr_mem_sz / 4))
		max_size = usr_mem_sz / 4;	/* only fourth mem by single test */

	Beg_brk_val = (long)sbrk(0);

	/*
	 * allow at least 4 times a big as current.
	 * This will override above code.
	 */
	if (max_size < Beg_brk_val * 4)	/* running on small mem and/or high # cpus */
		max_size = Beg_brk_val * 4;

	Max_brk_byte_size = max_size;

	TEST_PAUSE;

}

void cleanup()
{
	TEST_CLEANUP;

}

#else

int main()
{
	tst_brkm(TCONF, NULL, "test is not available on uClinux");
}

#endif /* if !defined(UCLINUX) */