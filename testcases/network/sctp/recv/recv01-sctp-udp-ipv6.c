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
 * Test Name: recv02-sctp-udp
 *
 * Test Description:
 *  Test 1: 
 *   check whether EBADF is returned when passed an invalid sockect 
 *   descriptor.
 *
 *  Test 2: 
 *   check whether ENOTSOCK is returned when passed an valid file 
 *   descriptor rather than a valid socket descriptor.
 * 
 *  Test 3: 
 *   check whether EFAULT is returned when passed an invalid address 
 *   pointer.
 *  
 *  Test 4: 
 *   check whether EINVAL is returned when passed an invalid address
 *   length.
 * 
 *  Test 5: 
 *   check whether MSG_PEEK is handled (no kernel hang).
 *  
 *  Test 6: 
 *   check whether MSG_WAITALL is handled (no kernel hang).
 * 
 *  Test 7: 
 *   check a regular recv behaves (no kernel hang). 
 *  
 *
 * Usage:  <for command-line>
 *  recv02-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
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
#include <signal.h>

#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

#define RUN_TIME	10


char *TCID="recv02-sctp-udp";		/* Test program identifier.    */
int testno;

char	buf[1024];
int	s;	/* socket descriptor */
struct sockaddr_in6 sin1;

void setup(void), setup0(void), setup1(void),
	cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int	domain;	/* PF_INET, PF_INET6, ... */
	int	type;	/* SOCK_STREAM, SOCK_DGRAM ... */
	int	proto;	/* protocol number (usually 0 = default) */
	void	*buf;	/* recv data buffer */
	int	buflen;	/* recv's 3rd argument */
	unsigned flags;	/* recv's 4th argument */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char *desc;
} tdat[] = {
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
		-1, EBADF, setup0, cleanup0, "bad file descriptor" },
	{ 0, 0, IPPROTO_SCTP, buf, sizeof(buf), 0,
		-1, ENOTSOCK, setup0, cleanup0, "invalid socket" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)-1, sizeof(buf), 0,
		-1, EFAULT, setup1, cleanup1, "invalid recv buffer" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), -1,
		-1, EINVAL, setup1, cleanup1, "invalid flags set" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), MSG_PEEK,
		0, 0, setup1, cleanup1, "MSG_PEEK flag" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), MSG_WAITALL,
		0, 0, setup1, cleanup1, "MSG_WAITALL flag" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
		0, 0, setup1, cleanup1, "a regular recv" },
};

int TST_TOTAL=sizeof(tdat)/sizeof(tdat[0]); /* Total number of test cases. */

extern int Tst_count;

void alarm_action();

struct sigaction new_act;


int
main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *)NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set alarm handler */
	new_act.sa_handler = alarm_action;
	sigaction(SIGALRM, &new_act, NULL);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno=0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();
	
			/* turn on the alarm */		
			alarm(RUN_TIME);
			TEST(recv(s, tdat[testno].buf, tdat[testno].buflen,
				tdat[testno].flags));
			if (TEST_RETURN > 0)
				TEST_RETURN = 0; /* all nonzero equal here */
			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				if (TEST_ERRNO == EINTR) {
					TEST_ERROR_LOG(TEST_ERRNO);
					tst_resm(TPASS, "%s test case got "
						"interrupted",
						tdat[testno].desc);
				} else {
					tst_resm(TFAIL, "%s ; returned"
					" %d (expected %d), errno %d (expected"
					" %d)", tdat[testno].desc,
					TEST_RETURN, tdat[testno].retval,
					TEST_ERRNO, tdat[testno].experrno);
				}
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TPASS, "%s successful",
					tdat[testno].desc);
			}
			tdat[testno].cleanup();
		}
	}
	cleanup();
}	/* End main */

pid_t pid;

void
setup(void)
{
	TEST_PAUSE;	/* if -P option specified */

	/* initialize sockaddr's */
	sin1.sin6_family = AF_INET6;
	sin1.sin6_port = htons((getpid() % 32768) +11000);
	sin1.sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;

	(void) signal(SIGPIPE, SIG_IGN);
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

/*
 * alarm_action() - reset the alarm.
 *
 */
void alarm_action() {

        new_act.sa_handler = alarm_action;
        sigaction (SIGALRM, &new_act, NULL);

        alarm(RUN_TIME);
        return;
}

