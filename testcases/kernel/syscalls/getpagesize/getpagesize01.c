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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

void setup();
void cleanup();

char *TCID = "getpagesize01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;

	int size, ret_sysconf;
	/***************************************************************
	 * parse standard options
	 ***************************************************************/
	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(getpagesize());

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "getpagesize failed");
			continue;	/* next loop for MTKERNEL */
		}

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

	cleanup();
	tst_exit();
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
}
