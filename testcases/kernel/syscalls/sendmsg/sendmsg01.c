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
 *
 */

/*
 * Test Name: sendmsg01
 *
 * Test Description:
 *  Verify that sendmsg() returns the proper errno for various failure cases
 *
 * Usage:  <for command-line>
 *  sendmsg01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	05/2003 Modified by Manoj Iyer - Make setup function set up lo device.
 *
 * RESTRICTIONS:
 *  None.
 *
 */

/* The #ifndef code below is for 2.5 64-bit kernels, where     */
/* the MSG_CMSG_COMPAT flag must be 0 in order for the syscall */
/* and this test to function correctly.                        */
#ifndef MSG_CMSG_COMPAT

#if defined (__powerpc64__) || defined (__mips64) || defined (__x86_64__) || defined (__sparc64__) || defined (__ia64__) || defined (__s390x__)
#define MSG_CMSG_COMPAT 0x80000000
#else
#define MSG_CMSG_COMPAT 0
#endif

#endif
/***************************************************/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

char *TCID = "sendmsg01";	/* Test program identifier.    */
int testno;

char buf[1024], bigbuf[128 * 1024];
int s;				/* socket descriptor */
struct sockaddr_in sin1, sin2;
struct sockaddr_un sun1;
struct msghdr msgdat;
char cbuf[4096];		/* control message buffer */
struct cmsghdr *control = 0;
int controllen = 0;
struct iovec iov[1];
static int sfd;			/* shared between do_child and start_server */
static int ufd;			/* shared between do_child and start_server */

void setup(void), setup0(void), setup1(void), setup2(void), setup3(void),
setup4(void), setup5(void), setup6(void), setup7(void), setup8(void),
cleanup(void), cleanup0(void), cleanup1(void), cleanup4(void), do_child(void);

struct test_case_t {		/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	struct iovec *iov;
	int iovcnt;		/* # elements in iovec */
	void *buf;		/* send data buffer */
	int buflen;		/* send buffer length */
	struct msghdr *msg;
	unsigned flags;
	struct sockaddr *to;	/* destination */
	int tolen;		/* length of "to" buffer */
	int retval;		/* syscall return value */
	int experrno;		/* expected errno */
	void (*setup) (void);
	void (*cleanup) (void);
	char *desc;
} tdat[] = {
/* 1 */
	{
	PF_INET, SOCK_STREAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, EBADF, setup0, cleanup0, "bad file descriptor"}
	,
/* 2 */
	{
	0, 0, 0, iov, 1, (void *)buf, sizeof(buf), &msgdat, 0,
		    (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, ENOTSOCK, setup0, cleanup0, "invalid socket"}
	,
/* 3 */
	{
	PF_INET, SOCK_DGRAM, 0, iov, 1, (void *)-1, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, EFAULT, setup1, cleanup1, "invalid send buffer"}
	,
/* 4 */
	{
	PF_INET, SOCK_STREAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sin2, sizeof(sin2),
		    0, EFAULT, setup5, cleanup1, "connected TCP"}
	,
/* 5 */
	{
	PF_INET, SOCK_STREAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, EPIPE, setup3, cleanup1, "not connected TCP"}
	,
/* 6 */
	{
	PF_INET, SOCK_DGRAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sin1, -1, -1,
		    EINVAL, setup1, cleanup1, "invalid to buffer length"},
/* 7 */
	{
	PF_INET, SOCK_DGRAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)-1, -1, -1, EINVAL,
		    setup1, cleanup1, "invalid to buffer"},
/* 8 */
	{
	PF_INET, SOCK_DGRAM, 0, iov, 1, (void *)bigbuf, sizeof(bigbuf),
		    &msgdat, 0,
		    (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, EMSGSIZE, setup1, cleanup1, "UDP message too big"}
	,
/* 9 */
	{
	PF_INET, SOCK_STREAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, EPIPE, setup2, cleanup1, "local endpoint shutdown"}
	,
/* 10 */
	{
	PF_INET, SOCK_STREAM, 0, 0, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, EFAULT, setup1, cleanup1, "invalid iovec pointer"}
	,
/* 11 */
	{
	PF_INET, SOCK_STREAM, 0, iov, 1, (void *)buf, sizeof(buf), 0,
		    0, (struct sockaddr *)&sin1, sizeof(sin1),
		    -1, EFAULT, setup1, cleanup1, "invalid msghdr pointer"}
	,
/* 12 */
	{
	PF_UNIX, SOCK_DGRAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sun1, sizeof(sun1),
		    0, 0, setup4, cleanup4, "rights passing"}
	,
/* 13 */
	{
	PF_UNIX, SOCK_DGRAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, ~MSG_CMSG_COMPAT, (struct sockaddr *)&sun1,
		    sizeof(sun1), -1, EOPNOTSUPP, setup4, cleanup4,
		    "invalid flags set w/ control"}
	,
/* 14 */
	{
	PF_INET, SOCK_STREAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, ~MSG_CMSG_COMPAT, (struct sockaddr *)&sin1,
		    sizeof(sin1), 0, EOPNOTSUPP, setup1, cleanup1,
		    "invalid flags set"}
	,
/* 15 */
	{
	PF_UNIX, SOCK_DGRAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sun1, sizeof(sun1),
		    0, EOPNOTSUPP, setup6, cleanup4, "invalid cmsg length"}
	,
/* 16 */
	{
	PF_UNIX, SOCK_DGRAM, 0, iov, 1, (void *)buf, sizeof(buf),
		    &msgdat, 0, (struct sockaddr *)&sun1, sizeof(sun1),
		    -1, EFAULT, setup8, cleanup4, "invalid cmsg pointer"}
,};

int TST_TOTAL = sizeof(tdat) / sizeof(tdat[0]);	/* Total number of test cases. */

int exp_enos[] =
    { EBADF, ENOTSOCK, EFAULT, EISCONN, ENOTCONN, EINVAL, EMSGSIZE, EPIPE,
ENOBUFS, 0 };


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
	maybe_run_child(&do_child, "dd", &sfd, &ufd);
#endif

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			iov[0].iov_base = tdat[testno].buf;
			iov[0].iov_len = tdat[testno].buflen;
			if (tdat[testno].type != SOCK_STREAM) {
				msgdat.msg_name = tdat[testno].to;
				msgdat.msg_namelen = tdat[testno].tolen;
			}
			msgdat.msg_iov = tdat[testno].iov;
			msgdat.msg_iovlen = tdat[testno].iovcnt;
			msgdat.msg_control = control;
			msgdat.msg_controllen = controllen;
			msgdat.msg_flags = 0;

			TEST(sendmsg(s, tdat[testno].msg, tdat[testno].flags));

			if (TEST_RETURN > 0)
				TEST_RETURN = 0;	/* all success equal */

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

pid_t start_server(struct sockaddr_in * sin0, struct sockaddr_un * sun0)
{
	struct sockaddr_in sin1 = *sin0;
	pid_t pid;

	/* set up inet socket */
	sfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		tst_brkm(TBROK, cleanup, "server socket failed: %s",
			 strerror(errno));
		return -1;
	}
	if (bind(sfd, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "server bind failed: %s",
			 strerror(errno));
		return -1;
	}
	if (listen(sfd, 10) < 0) {
		tst_brkm(TBROK, cleanup, "server listen failed: %s",
			 strerror(errno));
		return -1;
	}
	/* set up UNIX-domain socket */
	ufd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ufd < 0) {
		tst_brkm(TBROK, cleanup, "server UD socket failed: %s",
			 strerror(errno));
		return -1;
	}
	if (bind(ufd, (struct sockaddr *)sun0, sizeof(*sun0))) {
		tst_brkm(TBROK, cleanup, "server UD bind failed: %s",
			 strerror(errno));
		return -1;
	}

	switch ((pid = FORK_OR_VFORK())) {
	case 0:		/* child */
#ifdef UCLINUX
		if (self_exec(argv0, "dd", sfd, ufd) < 0) {
			tst_brkm(TBROK, cleanup, "server self_exec failed");
		}
#else
		do_child();
#endif
		break;
	case -1:
		tst_brkm(TBROK, cleanup, "server fork failed: %s",
			 strerror(errno));
		/* fall through */
	default:		/* parent */
		(void)close(sfd);
		(void)close(ufd);
		return pid;
	}

	  exit(1);
}

void do_child()
{
	struct sockaddr_in fsin;
	struct sockaddr_un fsun;
	fd_set afds, rfds;
	int nfds, cc, fd;

	FD_ZERO(&afds);
	FD_SET(sfd, &afds);
	FD_SET(ufd, &afds);

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
		if (FD_ISSET(ufd, &rfds)) {
			int newfd;

			fromlen = sizeof(fsun);
			newfd = accept(ufd, (struct sockaddr *)&fsun, &fromlen);
			if (newfd >= 0)
				FD_SET(newfd, &afds);
		}
		for (fd = 0; fd < nfds; ++fd) {
			if (fd != sfd && fd != ufd && FD_ISSET(fd, &rfds)) {
				cc = read(fd, buf, sizeof(buf));
				if (cc == 0 || (cc < 0 && errno != EINTR)) {
					(void)close(fd);
					FD_CLR(fd, &afds);
				}
			}
		}
	}
}

pid_t pid;
char tmpsunpath[1024];

void setup(void)
{

	int ret = 0;
	TEST_PAUSE;		/* if -P option specified */

	/* initialize sockaddr's */
	sin1.sin_family = AF_INET;
	sin1.sin_port = htons((getpid() % 32768) + 11000);
	sin1.sin_addr.s_addr = INADDR_ANY;

	tst_tmpdir();
	snprintf(tmpsunpath, 1024, "udsock%ld", (long)time(NULL));
	sun1.sun_family = AF_UNIX;
	strcpy(sun1.sun_path, tmpsunpath);

	/* this test will fail or in some cases hang if no eth or lo is
	 * configured, so making sure in setup that atleast lo is up
	 */
	ret = system("ifconfig lo up 127.0.0.1");
	if (WEXITSTATUS(ret) != 0) {
		tst_brkm(TBROK, cleanup,
			 "ifconfig failed to bring up loop back device");
		tst_exit();
	}

	pid = start_server(&sin1, &sun1);

	signal(SIGPIPE, SIG_IGN);
}

void cleanup(void)
{
	if (pid > 0)
		(void)kill(pid, SIGKILL);	/* kill server, if server exists */
	unlink(tmpsunpath);
	TEST_CLEANUP;
	tst_rmdir();

}

void setup0(void)
{
	if (tdat[testno].experrno == EBADF)
		s = 400;	/* anything not an open file */
	else if ((s = open("/dev/null", O_WRONLY)) == -1)
		tst_brkm(TBROK, cleanup, "error opening /dev/null - "
			 "errno: %s", strerror(errno));
}

void cleanup0(void)
{
	s = -1;
}

void setup1(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			 strerror(errno));
	}
	if (tdat[testno].type == SOCK_STREAM &&
	    connect(s, (struct sockaddr *)tdat[testno].to,
		    tdat[testno].tolen) < 0) {
		tst_brkm(TBROK, cleanup, "connect failed: %s", strerror(errno));
	}
}

void cleanup1(void)
{
	(void)close(s);
	s = -1;
}

void setup2(void)
{
	setup1();		/* get a socket in s */
	if (shutdown(s, 1) < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed connect "
			 "test %d: %s", testno, strerror(errno));
	}
}
void setup3(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			 strerror(errno));
	}
}

char tmpfilename[1024];
int tfd;

void setup4(void)
{

	setup1();		/* get a socket in s */

	(void)strcpy(tmpfilename, "sockXXXXXX");
	tfd = mkstemp(tmpfilename);
	if (tfd < 0) {
		tst_brkm(TBROK, cleanup4, "socket setup failed: %s",
			 strerror(errno));
	}
	control = (struct cmsghdr *)cbuf;
	memset(cbuf, 0x00, sizeof(cbuf));
	control->cmsg_len = sizeof(struct cmsghdr) + 4;
	control->cmsg_level = SOL_SOCKET;
	control->cmsg_type = SCM_RIGHTS;
	*(int *)CMSG_DATA(control) = tfd;
	controllen = control->cmsg_len;
}

void cleanup4(void)
{
	cleanup1();
	(void)close(tfd);
	tfd = -1;
	control = 0;
	controllen = 0;
}

void setup5(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed: %s",
			 strerror(errno));
	}
	if (connect(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "connect failed: %s", strerror(errno));
	}
	/* slight change destination (port) so connect() is to different
	 * 5-tuple than already connected
	 */
	sin2 = sin1;
	sin2.sin_port++;
}

void setup6(void)
{
	setup4();
/*
	controllen = control->cmsg_len = sizeof(struct cmsghdr) - 4;
*/
	controllen = control->cmsg_len = 0;
}

void setup7(void)
{
	setup4();
	controllen = 128 * 1024;
	control->cmsg_len = 0;
}

void setup8(void)
{
	setup4();
	control = (struct cmsghdr *)-1;
}