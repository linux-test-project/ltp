/*
 * Copyright (c) International Business Machines  Corp., 2005
 * Copyright (c) Wipro Technologies Ltd, 2005.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : getpagesize01
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Basic tests for getpagesize(2)
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Prashant P Yendigeri
 *                        <prashant.yendigeri@wipro.com>
 *			  Robbie Williamson
 *			  <robbiew@us.ibm.com>
 *
 *    DESCRIPTION
 *      This is a Phase I test for the getpagesize(2) system call.
 *      It is intended to provide a limited exposure of the system call.
 *
 **********************************************************/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "getpagesize01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_enos[] = { 0 };		/* must be a 0 terminated list */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int size, ret_sysconf;
	/***************************************************************
	 * parse standard options
	 ***************************************************************/
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(getpagesize());

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "getpagesize failed");
			continue;	/* next loop for MTKERNEL */
		}

		if (STD_FUNCTIONAL_TEST) {
			size = getpagesize();
			tst_resm(TINFO, "Page Size is %d", size);
			ret_sysconf = sysconf(_SC_PAGESIZE);
#ifdef DEBUG
			tst_resm(TINFO,
				 "Checking whether getpagesize returned same as sysconf");
#endif
			if (size == ret_sysconf)
				tst_resm(TPASS,
					 "getpagesize - Page size returned %d",
					 ret_sysconf);
			else
				tst_resm(TFAIL,
					 "getpagesize - Page size returned %d",
					 ret_sysconf);
		}
	}

	cleanup();

	tst_exit();
}

void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup()
{
	TEST_CLEANUP;
}