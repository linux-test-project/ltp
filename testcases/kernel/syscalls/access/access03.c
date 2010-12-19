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
/* $Id: access03.c,v 1.8 2009/03/23 13:35:39 subrata_modak Exp $ */
/**********************************************************
 *
 *	OS Test - Silicon Graphics, Inc.
 *
 *	TEST IDENTIFIER	: access03
 *
 *	EXECUTED BY	: anyone
 *
 *	TEST TITLE	: EFAULT error testing for access(2)
 *
 *	PARENT DOCUMENT	: acstds01
 *
 *	TEST CASE TOTAL	: 8
 *
 *	WALL CLOCK TIME	: 1
 *
 *	CPU TYPES		: ALL
 *
 *	AUTHOR		: Kathy Olmsted
 *
 *	CO-PILOT		: Tom Hampson
 *
 *	DATE STARTED	: 05/13/92
 *
 *	INITIAL RELEASE	: UNICOS 7.0
 *
 *	TEST CASES
 *
 *	access(2) test for errno(s) EFAULT.
 *
 *	INPUT SPECIFICATIONS
 *	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *	DURATION
 *	Terminates - with frequency and infinite modes.
 *
 *	SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *	ENVIRONMENTAL NEEDS
 *	  No run-time environmental needs.
 *
 *	DETAILED DESCRIPTION
 *
 *	Setup:
 *	  Setup signal handling.
 *		Make and change to a temporary directory.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno.
 *		If doing functional test
 *			check the errno returned and print result message
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *		Remove the temporary directory and exit.
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"

static void setup(void);
static void cleanup(void);

char *TCID = "access03";	/* Test program identifier.	*/
int TST_TOTAL = 8;		/* Total number of test cases. */

int exp_enos[] = { EFAULT, 0 };	/* List must end with 0 */

/* XXX (garrcoop): uh, this isn't a bad address yo. */
void *low_addr;
void *high_addr;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* set the expected errnos. */
	TEST_EXP_ENOS(exp_enos);

#define TEST_ACCESS(addr, mode) \
	if (access(low_addr, mode) == -1) { \
		if (errno == EFAULT) { \
			tst_resm(TPASS, \
			    "access(%p, %s) failed as expected with EFAULT", \
			    addr, #mode); \
		} else { \
			tst_resm(TFAIL|TERRNO, \
			    "access(%p, %s) failed unexpectedly; " \
			    "expected (EFAULT)", addr, #mode); \
		} \
	} else { \
		tst_resm(TFAIL, \
		    "access(%p, %s) succeeded unexpectedly", addr, #mode); \
	}

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST_ACCESS(low_addr, R_OK);
		TEST_ACCESS(low_addr, W_OK);
		TEST_ACCESS(low_addr, X_OK);
		TEST_ACCESS(low_addr, F_OK);

		TEST_ACCESS(high_addr, R_OK);
		TEST_ACCESS(high_addr, W_OK);
		TEST_ACCESS(high_addr, X_OK);
		TEST_ACCESS(high_addr, F_OK);

	}

	cleanup();
	tst_exit();

}

void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	low_addr = mmap(0, 1, PROT_NONE,
	    MAP_PRIVATE_EXCEPT_UCLINUX|MAP_ANONYMOUS, 0, 0);
	if (low_addr == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, NULL, "mmap failed");
	high_addr = get_high_address();
	if (high_addr == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "get_high_address failed");
	high_addr++;

	tst_tmpdir();
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	TEST_CLEANUP;

	tst_rmdir();
}

#else

int main()
{
	tst_brkm(TCONF, NULL, "test not available on UCLINUX");
}

#endif /* if !defined(UCLINUX) */