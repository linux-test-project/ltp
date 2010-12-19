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
 * Test Name: recvfrom01
 *
 * Test Description:
 *  Verify that recvfrom() returns the proper errno for various failure cases
 *
 * Usage:  <for command-line>
 *  recvfrom01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "recvfrom01";	/* Test program identifier.    */
int testno;

char buf[1024];
int s;				/* socket descriptor */
struct sockaddr_in sin1, from;
static int sfd;			/* shared between do_child and start_server */
socklen_t fromlen;

void do_child(void);
void setup(void);
void setup0(void);
void setup1(void);
void setup2(void);
void cleanup(void);
void cleanup0(void);
void cleanup1(void);
pid_t start_server(struct sockaddr_in *);

struct test_case_t {		/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	void *buf;		/* recv data buffer */
	size_t buflen;		/* recv's 3rd argument */
	unsigned flags;		/* recv's 4th argument */
	struct sockaddr *from;	/* from address */
	socklen_t *salen;	/* from address value/result buffer length */
	int retval;		/* syscall return value */
	int experrno;		/* expected errno */
	void (*setup) (void);
	void (*cleanup) (void);
	char *desc;
} tdat[] = {
/* 1 */
	{
	PF_INET, SOCK_STREAM, 0, buf, sizeof(buf), 0,
		    (struct sockaddr *)&from, &fromlen,
		    -1, EBADF, setup0, cleanup0, "bad file descriptor"},
/* 2 */
	{
	0, 0, 0, buf, sizeof(buf), 0, (struct sockaddr *)&from,
		    &fromlen, -1, ENOTSOCK, setup0, cleanup0, "invalid socket"},
/* 3 */
	{
	PF_INET, SOCK_STREAM, 0, (void *)buf, sizeof(buf), 0,
		    (struct sockaddr *)-1, &fromlen,
		    0, ENOTSOCK, setup1, cleanup1, "invalid socket buffer"},
/* 4 */
	{
	PF_INET, SOCK_STREAM, 0, (void *)buf, sizeof(buf), -1,
		    (struct sockaddr *)&from, &fromlen,
		    -1, EINVAL, setup2, cleanup1, "invalid socket length"},
/* 5 */
	{
	PF_INET, SOCK_STREAM, 0, (void *)-1, sizeof(buf), 0,
		    (struct sockaddr *)&from, &fromlen,
		    -1, EFAULT, setup1, cleanup1, "invalid recv buffer"},
/* 6 */
	{
PF_INET, SOCK_STREAM, 0, (void *)buf, sizeof(buf), -1,
		    (struct sockaddr *)&from, &fromlen,
		    -1, EINVAL, setup1, cleanup1, "invalid flags set"},};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);	/* Total number of test cases. */

int exp_enos[] = { EBADF, ENOTSOCK, EFAULT, EINVAL, 0 };


#ifdef UCLINUX
static char *argv0;
#endif

int main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

#ifdef UCLINUX
	argv0 = argv[0];
	maybe_run_child(&do_child, "d", &sfd);
#endif

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(recvfrom(s, tdat[testno].buf, tdat[testno].buflen,
				      tdat[testno].flags, tdat[testno].from,
				      tdat[testno].salen));
			if (TEST_RETURN >= 0)
				TEST_RETURN = 0;	/* all nonzero equal here */
			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				tst_resm(TFAIL, "%s ; returned"
					 " %ld (expected %d), errno %d (expected"
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

	tst_exit();
}

pid_t pid;

void setup(void)
{
	TEST_PAUSE;		/* if -P option specified */

	/* initialize sockaddr's */
	sin1.sin_family = AF_INET;
	sin1.sin_port = htons((getpid() % 32768) + 11000);
	sin1.sin_addr.s_addr = INADDR_ANY;
	pid = start_server(&sin1);
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
	fromlen = sizeof(from);
}

void cleanup0(void)
{
	s = -1;
}

void setup1(void)
{
	fd_set rdfds;
	struct timeval timeout;
	int n;

	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "socket() failed");
	if (tdat[testno].type == SOCK_STREAM &&
	    connect(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "connect failed");
	}
	/* Wait for something to be readable, else we won't detect EFAULT on recv */
	FD_ZERO(&rdfds);
	FD_SET(s, &rdfds);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	n = select(s + 1, &rdfds, 0, 0, &timeout);
	if (n != 1 || !FD_ISSET(s, &rdfds))
		tst_brkm(TBROK, cleanup,
			 "client setup1 failed - no message ready in 2 sec");
	fromlen = sizeof(from);
}

void setup2(void)
{
	setup1();
	fromlen = -1;
}

void cleanup1(void)
{
	(void)close(s);
	s = -1;
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
		if (self_exec(argv0, "d", sfd) < 0) {
			tst_brkm(TBROK, cleanup, "server self_exec failed");
		}
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

	  exit(1);
}

void do_child()
{
	struct sockaddr_in fsin;
	fd_set afds, rfds;
	int nfds, cc, fd;

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
			/* send something back */
			(void)write(newfd, "hoser\n", 6);
		}
		for (fd = 0; fd < nfds; ++fd)
			if (fd != sfd && FD_ISSET(fd, &rfds)) {
				cc = read(fd, buf, sizeof(buf));
				if (cc == 0 || (cc < 0 && errno != EINTR)) {
					(void)close(fd);
					FD_CLR(fd, &afds);
				}
			}
	}
}