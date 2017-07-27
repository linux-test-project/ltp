/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007                                   */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        newuname01.c                                           */
/*                                                                            */
/* Description: This tests the newuname() syscall                      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* newuname01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   newuname01                                             */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <unistd.h>
#include <sys/utsname.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "newuname01";
int testno;
int TST_TOTAL = 1;

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
void cleanup(void)
{

	tst_rmdir();

	tst_exit();
}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup(void)
{
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av)
{
	struct utsname name;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			TEST(ltp_syscall(__NR_uname, &name));
			if (TEST_RETURN == -1) {
				tst_brkm(TFAIL, cleanup, "%s failed - errno = %d : %s",
					 TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS,
					 "newuname call succeed: return value = %ld ",
					 TEST_RETURN);
				TEST(strcmp(name.sysname, "Linux"));	//Linux ?
				if (TEST_RETURN == 0) {
					tst_resm(TINFO, "This system is %s",
						 name.sysname);
					tst_resm(TINFO,
						 "The system infomation is :");
					tst_resm(TINFO,
						 "System is %s on %s hardware",
						 name.sysname, name.machine);

					tst_resm(TINFO, "Nodename is %s",
						 name.nodename);
					tst_resm(TINFO, "Version is %s, %s",
						 name.release, name.version);
					tst_resm(TINFO, "Domainname is %s ",
						 *(&name.machine + 1));
					cleanup();
					tst_exit();
				} else {
					tst_resm(TFAIL,
						 "%s failed - errno = %d : %s",
						 TCID, TEST_ERRNO,
						 strerror(TEST_ERRNO));
					tst_resm(TINFO,
						 "This system is not Linux");
					cleanup();
					tst_exit();
				}

			}

		}
	}
	tst_exit();
}
