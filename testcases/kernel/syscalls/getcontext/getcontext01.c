/*
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
 *    TEST IDENTIFIER   : getcontext01
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Basic tests for getcontext(2)
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Prashant P Yendigeri
 *                        <prashant.yendigeri@wipro.com>
 *
 *    DESCRIPTION
 *      This is a Phase I test for the getcontext(2) system call.
 *      It is intended to provide a limited exposure of the system call.
 *
 **********************************************************/

#include <features.h>
#if !defined(__UCLIBC__)

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ucontext.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "getcontext01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_enos[] = { 0 };		/* must be a 0 terminated list */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	ucontext_t ptr;
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(getcontext(&ptr));

		if (TEST_RETURN == -1)
			tst_resm(TFAIL|TTERRNO, "getcontext failed");
		else if (TEST_RETURN >= 0)
			tst_resm(TPASS, "getcontext passed");

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

#else /* systems that dont support obsolete getcontext */
int main()
{
	tst_brkm(TCONF, NULL, "system doesn't have getcontext support");
}
#endif