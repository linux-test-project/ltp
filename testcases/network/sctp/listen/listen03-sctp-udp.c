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
 * Test Name: listen01-sctp-udp
 *
 * Test Description:
 *      test 1:
 *      Check for EBADF. 
 *
 *      test 2:
 *      Check for ENOTSOCK.
 *      
 *      test 3:
 *      Check for default behaviors.
 *
 *      test 4-5: 
 *      Boundary tests. 
 *      
 *      test 6: 
 *      A regular listen. 
 *      
 * ALGORITHM
 *      test 1:
 *       s = 400.
 *
 *      test 2:
 *       s = 0.
 *      
 *      test 3:
 *       backlog = 0.
 *
 *      test 4-5: 
 *       backlog = -1;
 *       backlog = 1. 
 *      
 *      test 6: 
 *     	 backlog = 5.
 *
 * Usage:  <for command-line>
 *  listen01-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      02/2002 Adapted for SCTP by Robbie Williamson
 *	05/2002 Added in new test cases by Mingqin Liu
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/un.h>

#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

char *TCID="listen01-sctp-udp";		/* Test program identifier.    */
int testno;

int	s;	/* socket descriptor */

void setup(void), setup0(void), setup1(void),
	cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int	domain;	/* PF_INET, PF_UNIX, ... */
	int	type;	/* SOCK_STREAM, SOCK_DGRAM ... */
	int	proto;	/* protocol number (usually 0 = default) */
	int	backlog;	/* listen's 3rd argument */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char *desc;
} tdat[] = {
	{ 0, 0, IPPROTO_SCTP, 0, -1, EBADF, setup0, cleanup0,
		"bad file descriptor" },
	{ 0, 0, IPPROTO_SCTP, 0, -1, ENOTSOCK, setup0, cleanup0,
		"not a socket" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 0, 0, 0, setup1, cleanup1,
		"UDP-style listen: backlog = 0" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, -1, 0, 0, setup1, cleanup1,
		"backlog = -1" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 1, 0, 0, setup1, cleanup1,
		"backlog = 1" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 5, 0, 0, setup1, cleanup1,
		"UDP-style listen, backlog = 5" },
};

int TST_TOTAL=sizeof(tdat)/sizeof(tdat[0]); /* Total number of test cases. */


extern int Tst_count;

int
main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid;

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
			tdat[testno].setup();

			TEST(listen(s, tdat[testno].backlog));
			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				tst_resm(TFAIL, "%s ; returned"
					" %d (expected %d), errno %d (expected"
					" %d)", tdat[testno].desc,
					TEST_RETURN, tdat[testno].retval,
					TEST_ERRNO, tdat[testno].experrno);
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TPASS, "%s successful",
					tdat[testno].desc);
			}
			tdat[testno].cleanup();
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

void 
setup0(void)
{
	if (tdat[testno].experrno == EBADF)
		s = 400;	/* anything not an open file */
	else
		s = 0;		/* open, but not a socket */
}

void
cleanup0(void)
{
	s = -1;
}

void
setup1(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for listen: "
			"%s", strerror(errno));
	}
}

void
cleanup1(void)
{
	(void) close(s);
	s = -1;
}
