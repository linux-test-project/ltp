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
 * Test Name: send01-sctp-udp
 *
 * Test Description:
 *  Verify that send() returns the proper errno for various failure cases.
 * 	- EBADF
 *	- ENOTSOCK
 *	- EFAUTL
 *	- EINVAL
 *	- EAGAIN
 *
 *  Verify that send() would not hang or core dump when behaviors are 
 *  undefined in the spec. 
 *	- MSG_DONTWAIT
 *	- MSG_NOSIGNAL
 *
 * Usage:  <for command-line>
 *  send01-sctp-udp [-c n] [-i n] [-I x] [-p x] [-t]
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/un.h>

#include <netinet/in.h>

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"

char *TCID="send01-sctp-udp";		/* Test program identifier.    */
int testno;

char	buf[1024], bigbuf[128*1024];
int	s;	/* socket descriptor */

void setup(void), setup0(void), setup1(void), setup2(void),
	cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int	domain;	/* PF_INET, PF_UNIX, ... */
	int	type;	/* SOCK_STREAM, SOCK_DGRAM ... */
	int	proto;	/* protocol number (usually 0 = default) */
	void	*buf;	/* send data buffer */
	int	buflen;	/* send's 3rd argument */
	unsigned flags;	/* send's 4th argument */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char *desc;
} tdat[] = {
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
		-1, EBADF, setup0, cleanup0, "bad file descriptor" },
	{ 0, 0, IPPROTO_SCTP, buf, sizeof(buf), 0,
		-1, ENOTSOCK, setup0, cleanup0, "invalid socket" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)-1, sizeof(buf), 0,
                -1, EFAULT, setup1, cleanup1, "invalid send buffer" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), -1,
                -1, EFAULT, setup1, cleanup1, "invalid flags set" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
               -1, EINVAL, setup1, cleanup1, "a regular send" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
               -1, EAGAIN, setup2, cleanup1, "EAGAIN" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), MSG_DONTWAIT,
               -1, 22, setup1, cleanup1, "MSG_DONTWAIT flag" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), MSG_NOSIGNAL,
               -1, 22, setup1, cleanup1, "MSG_NOSIGNAL flag" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
               -1, 22, setup1, cleanup1, "a regular send" },
};

int TST_TOTAL=sizeof(tdat)/sizeof(tdat[0]); /* Total number of test cases. */

extern int Tst_count;

int
main(int ac, char *av[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *)NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		Tst_count = 0;

		for (testno=0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(send(s, 
				  tdat[testno].buf, 
				  tdat[testno].buflen, 
				  tdat[testno].flags));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != tdat[testno].experrno) {
				if ( EINVAL == TEST_ERRNO 
				     && EAGAIN == tdat[testno].experrno) {
					tst_resm(TPASS, "%s successful",
					tdat[testno].desc);

				} 
				else {
					tst_resm(TFAIL, "%s ; returned"
					" %d (expected %d), errno %d (expected"
					" %d)", tdat[testno].desc,
					TEST_RETURN, tdat[testno].retval,
					TEST_ERRNO, tdat[testno].experrno);
				}
			} else {
				tst_resm(TPASS, "%s successful",
					tdat[testno].desc);
			}
			tdat[testno].cleanup();
		}
	}
	cleanup();

	/*NOTREACHED*/
}

pid_t pid;

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
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			strerror(errno));
	}
}

void
cleanup1(void)
{
	(void) close(s);
	s = -1;
}

void
setup2(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			strerror(errno));
	}

	if (set_nonblock(s) < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			strerror(errno));

	}	
}

