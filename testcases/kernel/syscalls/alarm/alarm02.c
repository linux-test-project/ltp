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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
/* $Id: alarm02.c,v 1.4 2009/08/28 10:57:29 vapier Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER   : alarm02
 *
 *    TEST TITLE        : Boundary Value Test for alarm(2)
 *
 *    PARENT DOCUMENT   : almtds02
 *
 *    TEST CASE TOTAL   : 3
 *
 *    WALL CLOCK TIME   : 1
 *
 *    CPU TYPES         : ALL
 *
 *    AUTHOR            : Billy Jean Horne
 *
 *    CO-PILOT          : Kathy Olmsted
 *
 *    DATE STARTED      : 06/01/92
 *
 *    INITIAL RELEASE   : UNICOS 7.0
 *
 *    TEST CASES
 *      Test Case One - A call to alarm() shall not return an error if
 *       seconds is a -1.
 *       Test FAILS if a non-zero value is returned.
 *      Test Case Two - A call to alarm() shall not return an error if
 *       seconds is the maximum unsigned integer (2**63).
 *       Test FAILS if a non-zero value is returned.
 *      Test Case Three - A call to alarm() shall not return an error if
 *       seconds is the maximum unsigned integer plus 1 ((2**63)+1).
 *       Test FAILS if a non-zero value is returned.
 *
 *    ENVIRONMENTAL NEEDS
 *      The libcuts.a and libsys.a libraries must be included in
 *      the compilation of this test.
 *
 *    DETAILED DESCRIPTION
 *
 *      Setup:
 *        Define a cleanup function.
 *
 *      Test:
 *       Loop for each test case.
 *        Execute alarm (0) system call to clear previous alarm.
 *        Check return code, if system call failed (return=-1)
 *           Issue a FAIL message and exit the test.
 *        Call alarm() with boundary values for seconds.
 *        Verify that returned value is as expected.
 *        Report results.
 *
 *      Cleanup:
 *
 */
#include <sys/types.h>
#include <errno.h>
#include <sys/signal.h>
#include <limits.h>
#include "test.h"

void setup(void);
void cleanup(void);
void alarm_received(int sig);

char *TCID = "alarm02";
int TST_TOTAL = 3;

int received_alarm = 0;		/* Indicates a SIGALRM was received */

int main(int ac, char **av)
{

	/* Parameters for usc code  */
	int lc;

	/* Parameters for alarm test */
	char *buf[] = { "-1", "ULONG_MAX", "ULONG_MAX+1" };
	unsigned long int sec[] = { -1, ULONG_MAX, ULONG_MAX + 1 };
	int exp[] = { 0, 0, 0 };
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			received_alarm = 0;
			signal(SIGALRM, alarm_received);

			TEST(alarm(sec[i]));
			alarm(0);
			if (TEST_RETURN != 0) {
				tst_resm(TFAIL,
					 "alarm(%lu) returned %ld, when %u was "
					 "expected for value %s",
					 sec[i], TEST_RETURN, exp[i], buf[i]);
			} else {
				if (received_alarm == 1) {
					tst_resm(TFAIL,
						 "alarm(%lu) returned %ldu but an "
						 "alarm signal was received for "
						 "value %s",
						 sec[i], TEST_RETURN, buf[i]);
				} else {
					tst_resm(TPASS,
						 "alarm(%lu) returned %ld as "
						 "expected for value %s",
						 sec[i], TEST_RETURN, buf[i]);
				}

			}
		}
		/*
		 *  Reset alarm before cleanup.
		 */

		alarm(0);

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

void alarm_received(int sig)
{
	received_alarm = 1;
}
