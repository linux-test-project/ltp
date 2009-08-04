/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 * Test Name: socketpair01
 *
 * Test Description:
 *  Verify that socketpair() returns the proper errno for various failure cases
 *
 * Usage:  <for command-line>
 *  socketpair01 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

char *TCID = "socketpair01";	/* Test program identifier.    */
int testno;
int exp_enos[] = { EINVAL, EPERM, EFAULT, EOPNOTSUPP, EPROTONOSUPPORT, 0 };

int sv[2];

void setup(void), cleanup(void);

struct test_case_t {		/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	int *sv;		/* socket descriptor vector */
	int retval;		/* syscall return value */
	int experrno;		/* expected errno */
	char *desc;
} tdat[] = {
	{
	0, SOCK_STREAM, 0, sv, -1, EAFNOSUPPORT, "invalid domain"}, {
	PF_INET, 75, 0, sv, -1, EINVAL, "invalid type"}, {
	PF_UNIX, SOCK_DGRAM, 0, sv, 0, 0, "UNIX domain dgram"}, {
	PF_INET, SOCK_RAW, 0, sv, -1, ESOCKTNOSUPPORT,
		    "raw open as non-root"},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	{
	PF_UNIX, SOCK_STREAM, 0, 0, -1, EFAULT, "bad aligned pointer"}, {
	PF_UNIX, SOCK_STREAM, 0, (int *)7, -1, EFAULT, "bad unaligned pointer"},
#endif
	{
	PF_INET, SOCK_DGRAM, 17, sv, -1, EOPNOTSUPP, "UDP socket"}, {
	PF_INET, SOCK_DGRAM, 6, sv, -1, ESOCKTNOSUPPORT, "TCP dgram"}, {
	PF_INET, SOCK_STREAM, 6, sv, -1, EOPNOTSUPP, "TCP socket"}, {
PF_INET, SOCK_STREAM, 1, sv, -1, ESOCKTNOSUPPORT, "ICMP stream"},};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);	/* Total number of test cases. */

extern int Tst_count;

int main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int s;

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			TEST((s = socketpair(tdat[testno].domain,
					     tdat[testno].type,
					     tdat[testno].proto,
					     tdat[testno].sv)));
			if (TEST_RETURN >= 0) {
				TEST_RETURN = 0;	/* > 0 equivalent */
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
			}
			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN &&
			     (TEST_ERRNO != tdat[testno].experrno
			      && TEST_ERRNO != EPROTONOSUPPORT))) {
				tst_resm(TFAIL,
					 "%s ; returned"
					 " %d (expected %d), errno %d (expected"
					 " %d)", tdat[testno].desc, s,
					 tdat[testno].retval, TEST_ERRNO,
					 tdat[testno].experrno);
			} else {
				tst_resm(TPASS, "%s successful",
					 tdat[testno].desc);
			}
			(void)close(s);
		}
	}

	cleanup();

	return 0;

}				/* End main */

void setup(void)
{
	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;		/* if -P option specified */
}

void cleanup(void)
{
	TEST_CLEANUP;
	tst_exit();
}
