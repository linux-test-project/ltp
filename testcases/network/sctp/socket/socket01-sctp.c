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
 * Test Name: socket01-sctp-udp
 *
 * Test Description:
 *  Test 1:
 *   PF_INET/SOCK_SEQPACKET combination test.
 *  
 *  Test 2:
 *   PF_INET6/SOCK_SEQPACKET combination test.
 *
 *  Test 3:
 *   PF_INET/SOCK_STREAM combination test.
 *
 *  Test 4:
 *   PF_INET6/SOCK_STREAM combination test.
 *
 *  Test 5:
 *   Expect EAFNOSUPPORT when passed an inalid family value.
 *
 *  Test 6:
 *   Expect EINVAL when passed an inalid type.
 *
 * Usage:  <for command-line>
 *  socket01-sctp-udp [-c n] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *	02/2002 Robbie Williamson
 *		-Adapted to SCTP
 *      04/2002 Mingqin Liu
 *             -Added in IPv6 test cases.
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

char *TCID="socket01-sctp-udp";		/* Test program identifier.    */
int testno;

void setup(void), cleanup(void);

struct test_case_t {		/* test case structure */
	int	domain;	/* PF_INET, PF_INET6, ... */
	int	type;	/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;	/* protocol number (usually 0 = default) */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	char *desc;
} tdat[] = {
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 0, 0, "SCTP UDP-style packet" },
	{ PF_INET, SOCK_STREAM, IPPROTO_SCTP, 0, 0, "SCTP TCP-style packet" },
       	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 0, 0, "IPv6 SCTP UDP-style packet" },
	{ PF_INET6, SOCK_STREAM, IPPROTO_SCTP, 0, 0, "IPv6 SCTP TCP-style packet" },
        { 0, SOCK_SEQPACKET, 0, -1, EAFNOSUPPORT, "invalid domain" },
        { PF_INET, 75, 0, -1, EINVAL, "invalid type" },
};

int TST_TOTAL=sizeof(tdat)/sizeof(tdat[0]); /* Total number of test cases. */

extern int Tst_count;

int
main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int	s;

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *) NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno=0; testno < TST_TOTAL; ++testno) {
			TEST(s=socket(tdat[testno].domain, tdat[testno].type,
				tdat[testno].proto));
			if (TEST_RETURN >= 0) {
				TEST_RETURN = 0;	/* > 0 equivalent */
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
			}
			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				tst_resm(TFAIL, "%s ; returned"
					" %d (expected %d), errno %d (expected"
					" %d)", tdat[testno].desc,
					s, tdat[testno].retval,
					TEST_ERRNO, tdat[testno].experrno);
			} else {
				tst_resm(TPASS, "%s successful",
					tdat[testno].desc);
			}
			(void) close(s);
		}
	}
	cleanup();
	/*NOTREACHED*/
}	/* End main */

void
setup(void)
{

	TEST_PAUSE;	/* if -P option specified */
}

void
cleanup(void)
{
	TEST_CLEANUP;
	tst_exit();
}

