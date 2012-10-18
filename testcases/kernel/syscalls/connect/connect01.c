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
 * Test Name: connect01
 *
 * Test Description:
 *  Verify that connect() returns the proper errno for various failure cases
 *
 * Usage:  <for command-line>
 *  connect01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
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

char *TCID = "connect01";	/* Test program identifier.    */
int testno;

int s, s2;			/* socket descriptor */
struct sockaddr_in sin1, sin2, sin3, sin4;
static int sfd;			/* shared between start_server and do_child */

void setup(void), setup0(void), setup1(void), setup2(void),
cleanup(void), cleanup0(void), cleanup1(void), do_child(void);

static pid_t start_server(struct sockaddr_in *);

struct test_case_t {		/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	struct sockaddr *sockaddr;	/* socket address buffer */
	int salen;		/* connect's 3rd argument */
	int retval;		/* syscall return value */
	int experrno;		/* expected errno */
	void (*setup) (void);
	void (*cleanup) (void);
	char *desc;
} tdat[] = {
	{
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin1,
		    sizeof(struct sockaddr_in), -1, EBADF, setup0,
		    cleanup0, "bad file descriptor"},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	{
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)-1,
		    sizeof(struct sockaddr_in), -1, EFAULT, setup1,
		    cleanup1, "invalid socket buffer"},
#endif
	{
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin1,
		    3, -1, EINVAL, setup1, cleanup1, "invalid salen"}, {
	0, 0, 0, (struct sockaddr *)&sin1,
		    sizeof(sin1), -1, ENOTSOCK, setup0, cleanup0,
		    "invalid socket"}
	, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin1,
		    sizeof(sin1), -1, EISCONN, setup2, cleanup1,
		    "already connected"}
	, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin2,
		    sizeof(sin2), -1, ECONNREFUSED, setup1, cleanup1,
		    "connection refused"}
	, {
	PF_INET, SOCK_STREAM, 0, (struct sockaddr *)&sin4,
		    sizeof(sin4), -1, EAFNOSUPPORT, setup1, cleanup1,
		    "invalid address family"}
,};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);	/* Total number of test cases. */

int exp_enos[] = { EBADF, EFAULT, EINVAL, ENOTSOCK, EISCONN, ECONNREFUSED,
	EAFNOSUPPORT, 0
};


#ifdef UCLINUX
static char *argv0;
#endif

int main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}
#ifdef UCLINUX
	argv0 = argv[0];
	maybe_run_child(&do_child, "d", &sfd);
#endif

	/* set up expected errnos */
	TEST_EXP_ENOS(exp_enos);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(connect
			     (s, tdat[testno].sockaddr, tdat[testno].salen));
			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				tst_resm(TFAIL, "%s ; returned"
					 " %ld (expected %d), errno %d (expected"
					 " %d)", tdat[testno].desc,
					 TEST_RETURN, tdat[testno].retval,
					 TEST_ERRNO, tdat[testno].experrno);
			} else {
				tst_resm(TPASS, "%s successful",
					 tdat[testno].desc);
			}
			tdat[testno].cleanup();
		}
	}
	cleanup();

	tst_exit();
 }

pid_t pid;

void setup(void)
{
	TEST_PAUSE;		/* if -p option specified */

	/* initialize sockaddr's */
	sin1.sin_family = AF_INET;
	sin1.sin_port = htons((getpid() % 32768) + 11000);
	sin1.sin_addr.s_addr = INADDR_ANY;
	pid = start_server(&sin1);

	sin2.sin_family = AF_INET;
	/* this port must be unused! */
	sin2.sin_port = htons((getpid() % 32768) + 10000);
	sin2.sin_addr.s_addr = INADDR_ANY;

	sin3.sin_family = AF_INET;
	sin3.sin_port = 0;
	/* assumes no route to this network! */
	sin3.sin_addr.s_addr = htonl(0x0AFFFEFD);

	sin4.sin_family = 47;	/* bogus address family */
	sin4.sin_port = 0;
	sin4.sin_addr.s_addr = htonl(0x0AFFFEFD);

}

void cleanup(void)
{
	(void)kill(pid, SIGKILL);	/* kill server */

	TEST_CLEANUP;

}

void setup0(void)
{
	if (tdat[testno].experrno == EBADF)
		s = 400;	/* anything not an open file */
	else if ((s = open("/dev/null", O_WRONLY)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open(/dev/null) failed");

}

void cleanup0(void)
{
	s = -1;
}

void setup1(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "socket() failed");
}

void cleanup1(void)
{
	(void)close(s);
	s = -1;
}

void setup2(void)
{
	setup1();		/* get a socket in s */
	if (connect(s, (const struct sockaddr *)&sin1, sizeof(sin1)) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "socket setup failed connect test %d", testno);
}

pid_t start_server(struct sockaddr_in *sin0)
{
	struct sockaddr_in sin1 = *sin0;
	pid_t pid;

	sfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "server socket failed");
		return -1;
	}
	if (bind(sfd, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "server bind failed");
		return -1;
	}
	if (listen(sfd, 10) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "server listen failed");
		return -1;
	}
	switch ((pid = FORK_OR_VFORK())) {
	case 0:		/* child */
#ifdef UCLINUX
		self_exec(argv0, "d", sfd);
#else
		do_child();
#endif
		break;
	case -1:
		tst_brkm(TBROK|TERRNO, cleanup, "server fork failed");
		/* fall through */
	default:		/* parent */
		(void)close(sfd);
		return pid;
	}

	return -1;
}

void do_child()
{
	struct sockaddr_in fsin;
	fd_set afds, rfds;
	int nfds, cc, fd;
	char c;

	FD_ZERO(&afds);
	FD_SET(sfd, &afds);

	nfds = getdtablesize();

	/* accept connections until killed */
	while (1) {
		socklen_t fromlen;

		memcpy(&rfds, &afds, sizeof(rfds));

		if (select(nfds, &rfds, (fd_set *) 0, (fd_set *) 0,
			   (struct timeval *)0) < 0)
			if (errno != EINTR)
				exit(1);
		if (FD_ISSET(sfd, &rfds)) {
			int newfd;

			fromlen = sizeof(fsin);
			newfd = accept(sfd, (struct sockaddr *)&fsin, &fromlen);
			if (newfd >= 0)
				FD_SET(newfd, &afds);
		}
		for (fd = 0; fd < nfds; ++fd)
			if (fd != sfd && FD_ISSET(fd, &rfds)) {
				if ((cc = read(fd, &c, 1)) == 0) {
					(void)close(fd);
					FD_CLR(fd, &afds);
				}
			}
	}
}
