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
 * Test Name: recvfrom01-sctp-udp
 *
 * Test Description:
 *  Test 1: 
 *   Check whether EBADF is returned when recvfrom() is called with an 
 *   invalid socket descriptor.
 *
 *  Test 2: 
 *   Check whether ENOTSOCK is returned when recvfrom() is called with a 
 *   valid file descriptor rather than a valid socket descriptor. 
 *
 *  Test 3: 
 *   Chech whether EFAULT is returned when recvfrom() is called with an
 *   invalid sockaddr pointer.
 *
 *  Test 4: 
 *   Check whether EINVAL is returned when recvfrom() is called with an
 *   inalid sockaddr length.
 * 
 *  Test 5: 
 *   Check whether EFAULT is returned when recvfrom() is called with an
 *   invalid receive buffer.
 *
 *  Test 6: 
 *   Check whether EINVAL is returned when recvfrom() is called with an 
 *   invalid flag.
 *
 *  Test 7: 
 *   Check whether two consecutive recvfrom() calls return the same message
 *   when MSG_PEEK flag is set.
 * 
 *  Test 8: 
 *   Check whether MSG_WAITALL is accepted.  
 *
 *  Test 9: 
 *   Check whether a regular recvfrom() behaves as expected.
 *
 * Algorithm:
 *
 *  Create a server out of each test case except that all of the servers 
 *  share the same port and bind address and that only one of the servers 
 *  is alive at any given time.  
 *  
 *  A client is created and keeps shooting packages at the servers. At the 
 *  server end, messages are processed or discarded accordingly.  
 *  
 * Usage:  <for command-line>
 *  recvfrom01-sctp-udp-ipv4 [-c n] [-i n] [-I x] [-P x] [-t]
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
#include <unistd.h>
#include <netinet/in.h>

#include <sys/ipc.h>
#include <sys/sem.h>

#include "test.h"
#include "usctest.h"

#define PORT	10001
#define MESSAGE "THIS IS FOR RECVFROM"

char *TCID="recvfrom01-sctp-udp-ipv4";	/* Test program identifier.    */
int testno;

char	buf[1024];
int	s, cfd;	/* socket descriptor */
struct sockaddr_in sin1, from, to;
int	fromlen;
pid_t   pid;

void setup(void), setup0(void), setup1(void), setup2(void),
	cleanup(void), cleanup0(void), cleanup1(void);

pid_t start_client(void);

struct test_case_t {		/* test case structure */
	int	domain;	/* PF_INET, PF_UNIX, ... */
	int	type;	/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;	/* protocol number (usually 0 = default) */
	void	*buf;	/* recvfrom data buffer */
	int	buflen;	/* recvfrom's 3rd argument */
	unsigned flags;	/* recvfrom's 4th argument */
	struct sockaddr *from;	/* from address */
	int	*salen;	/* from address value/result buffer length */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char *desc;
} tdat[] = {
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
		(struct sockaddr *)&from, &fromlen,
		-1, EBADF, setup0, cleanup0, "bad file descriptor" },
	{ 0, 0, IPPROTO_SCTP, buf, sizeof(buf), 0, (struct sockaddr *)&from, 
		&fromlen, -1, ENOTSOCK, setup0, cleanup0, "invalid socket" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)buf, sizeof(buf), 0,
		(struct sockaddr *)-1, &fromlen,
		-1, EFAULT, setup1, cleanup1, "invalid socket buffer" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)buf, sizeof(buf), 0,
		(struct sockaddr *)&from, &fromlen,
		-1, EINVAL, setup2, cleanup1, "invalid socket length" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)-1, sizeof(buf), 0,
		(struct sockaddr *)&from, &fromlen,
		-1, EFAULT, setup1, cleanup1, "invalid recv buffer" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)buf, sizeof(buf), -1,
		(struct sockaddr *)&from, &fromlen,
		-1, EINVAL, setup1, cleanup1, "invalid flags set" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)buf, sizeof(buf), 
		MSG_PEEK, (struct sockaddr *)&from, &fromlen,
		0, 0, setup1, cleanup1, "MSG_PEEK" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)buf, sizeof(buf), 
		MSG_WAITALL, (struct sockaddr *)&from, &fromlen,
		0, 0, setup1, cleanup1, "MSG_WAITALL" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)buf, sizeof(buf), 0,
		(struct sockaddr *)&from, &fromlen,
		0, 0, setup1, cleanup1, "a regular recvfrom" },
};

int TST_TOTAL=sizeof(tdat)/sizeof(tdat[0]); /* Total number of test cases. */

extern int Tst_count;

int
main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int		msglen; 

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *)NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno=0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(recvfrom(s, tdat[testno].buf, tdat[testno].buflen,
				tdat[testno].flags, tdat[testno].from,
				tdat[testno].salen));

			if (TEST_RETURN >= 0) {
				msglen = TEST_RETURN;
				TEST_RETURN = 0; /* all nonzero equal here */
			}
			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				tst_resm(TFAIL, "%s ; returned"
					" %d (expected %d), errno %d (expected"
					" %d)", tdat[testno].desc,
					TEST_RETURN, tdat[testno].retval,
					TEST_ERRNO, tdat[testno].experrno);
			} else {
				char local_buf[1024];
			
				/* 
				 * If it is a MSG_PEEK, call recvmsg once 
				 * more to get another message back for 
				 * comparison.  
				 */
				if (tdat[testno].flags != MSG_PEEK 
				    || (tdat[testno].flags == MSG_PEEK  
				 	&& msglen == recvfrom(s, local_buf, 
						tdat[testno].buflen,
						tdat[testno].flags, 
						tdat[testno].from,
						tdat[testno].salen)
					&& !memcmp(tdat[testno].buf, local_buf,
						 msglen))) {
						TEST_ERROR_LOG(TEST_ERRNO);
						tst_resm(TPASS, "%s successful",
							tdat[testno].desc);
				} else {
					tst_resm(TFAIL, "%s; returned"
						" %d (expected %d),"
						" errno %d (expected"
						" %d)", tdat[testno].desc,
						TEST_RETURN, 
						tdat[testno].retval,
						TEST_ERRNO, 
						tdat[testno].experrno);

				}
			}
			tdat[testno].cleanup();
		}
	}
	cleanup();
}	/* End main */


void
setup(void)
{
	TEST_PAUSE;	/* if -P option specified */

	/* initialize sockaddr's */
	sin1.sin_family = AF_INET;
	sin1.sin_port = htons(PORT);
	sin1.sin_addr.s_addr = INADDR_ANY;
	pid = start_client(); 
}

void
cleanup(void)
{
	(void) kill(pid, SIGKILL);	/* kill server */
	close(cfd);
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
	fromlen = sizeof(from);
	
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
	fromlen = sizeof(from);

	if (bind(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "server bind failed: %s",
			strerror(errno));
		return;
	}
	if (listen(s, 10) < 0) {
		tst_brkm(TBROK, cleanup, "server listen failed: %s",
			strerror(errno));
		return;
	}

}

void
setup2(void)
{
	setup1();
	fromlen = -1;
}

void
cleanup1(void)
{
	(void) close(s);
	s = -1;
}


/* 
 * Fork a child and keep the child sending messages out to the 
 * parent process. 
 */
pid_t
start_client(void) 
{
	struct sockaddr_in local;

	
	cfd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	if (cfd < 0) {
		tst_brkm(TBROK, cleanup, "client socket failed: %s",
			strerror(errno));
		exit(1);
	}

	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = inet_addr("127.0.0.1");
	to.sin_port = htons(PORT);

	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(0);

        if (bind(cfd, (struct sockaddr *)&local, sizeof(local)) < 0) {
                tst_brkm(TBROK, cleanup, "client bind failed: %s",
                        strerror(errno));
                exit(1);
        }

	switch ((pid = fork())) {
	case 0:	/* child */
		
		break;
	case -1:
		tst_brkm(TBROK, cleanup, "server fork failed: %s",
			strerror(errno));
		/* fall through */
	default: /* parent */
		(void) close(cfd);
		return pid;
	}

	while (1) {
		sleep(1);

		/* send something back */
		if( sendto(cfd, MESSAGE, sizeof(MESSAGE)+1, 0,
				(struct sockaddr*) &to, sizeof(to)) <0) {
		printf("sendto error:%i\n", errno);
		};
	}
}

