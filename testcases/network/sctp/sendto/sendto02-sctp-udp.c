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
 * Test Name: sendto02-sctp-udp
 *
 * Test Description:
 *  Test 1:
 *   Check whether EBADF is returned when sendto() is called with an
 *   invalid socket descriptor.
 *
 *  Test 2:
 *   Check whether ENOTSOCK is returned when sendto() is called with a
 *   valid file descriptor rather than a valid socket descriptor.
 *
 *  Test 3:
 *   Chech whether EFAULT is returned when sendto() is called with an
 *   invalid send buffer.
 *
 *  Test 4:
 *   Check whether EINVAL is returned when sendto() is called with an
 *   sock addr length.
 *
 *  Test 5: 
 *   Check whether EFAULT is returned when sendto() is called with an
 *   invalid sock addr pointer.
 *  
 *  Test 6: 
 *   Check whether EINVAL is returned when sendto() is called with an
 *   invalid flag.
 *  
 *  Test 7: 
 *   Check whether EAGAIN is returned when sendto() is called upon a
 *   non-blocking socket.
 *  
 *  Test 8: 
 *   Check whether MSG_DONTWAIT flag is accepted.
 *  
 *  Test 9: 
 *   Check whether MSG_NOSIGNAL is accepted.
 *  
 *  Test 10: 
 *   Check whether a regular sendto() behaves as expected.
 *
 *
 * Usage:  <for command-line>
 *  sendto02-sctp-udp [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
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
 * NOTE
 *  O_NONBLOCK and MSG_DONTWAIT do the same thing. 
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <netinet/sctp.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"
#define PORT 10001

char *TCID="sendto02-sctp-udp";		/* Test program identifier.    */
int testno;

char	buf[]="This is sendto", bigbuf[128*1024];
int	s;	/* socket descriptor */
struct sockaddr_in6 sin1, server_addr;

void setup(void), setup0(void), setup1(void), setup2(void), 
	cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int	domain; /* PF_INET, PF_UNIX, ... */
	int	type;	/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;	/* protocol number (usually 0 = default) */
	void	*buf;	/* sendto data buffer */
	int	buflen;	/* sendto's 3rd argument */
	unsigned flags;	/* sendto's 4th argument */
	struct sockaddr_in6 *to;	/* destination */
	int	tolen;		/* length of "to" buffer */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char *desc;
} tdat[] = {
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0, 
		&sin1, sizeof(sin1), -1, EBADF, setup0, cleanup0, 
		"bad file descriptor" },
	{ 0, 0, IPPROTO_SCTP, buf, sizeof(buf), 0, &sin1, sizeof(sin1),
		-1, ENOTSOCK, setup0, cleanup0, "invalid socket" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, (void *)-1, sizeof(buf), 0, 
		&sin1, sizeof(sin1), -1, EFAULT, setup1, cleanup1, 
		"invalid send buffer" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0, 
		&sin1, -1, -1, EINVAL, setup1, cleanup1, 
		"invalid to buffer length" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0, 
		(struct sockaddr_in6 *)-1, sizeof(sin1),
		-1, EFAULT, setup1, cleanup1, "invalid to buffer" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), -1, 
		&sin1, sizeof(sin1), 0, EINVAL, setup1, cleanup1, 
		"invalid flags set" },
        { PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0,
		(struct sockaddr_in6 *)&sin1, sizeof(sin1),
               -1, EAGAIN, setup2, cleanup1, "EAGAIN" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 
		MSG_DONTWAIT, &sin1, sizeof(sin1), 0, 0, setup1, cleanup1, 
		"MSG_DONTWAIT flag" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 
		MSG_NOSIGNAL, &sin1, sizeof(sin1), 0, 0, setup1, cleanup1, 
		"MSG_NOSIGNAL flag" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, buf, sizeof(buf), 0, 
		&sin1, sizeof(sin1), 0, 0, setup1, cleanup1, 
		"a regular sendto" },
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

	setup();


	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		Tst_count = 0;
		for (testno=0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(sendto(s, tdat[testno].buf, 
				tdat[testno].buflen,
				tdat[testno].flags, 
				(struct sockaddr *) tdat[testno].to,
				tdat[testno].tolen));

			if (TEST_RETURN > 0)
				TEST_RETURN = 0;	/* all success equal */

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_RETURN != tdat[testno].retval ||
			    (TEST_RETURN < 0 &&
			     TEST_ERRNO != tdat[testno].experrno)) {
				tst_resm(TFAIL, "%s ; returned"
					" %d (expected %d), errno %d (expected"
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

	/*NOTREACHED*/
}


void
setup(void)
{
	int count, i, got_addr;
	char v6_local_host[64];
        local_addr_t    local_addrs[10];

	TEST_PAUSE;	/* if -P option specified */

	/* initialize sockaddr's */
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_port = htons(PORT);
	server_addr.sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;
	start_server(&server_addr);

	/* look for an IPv6 address to bind to */

        get_ip_addresses(local_addrs, &count);

	got_addr = 0;
	i = 0;
        while (i < count && !got_addr)
        {
                if ((strcmp(local_addrs[i].if_name, "lo"))
                     && local_addrs[i].has_v6) {
                        strcpy(v6_local_host, local_addrs[i].v6_addr);  
                        got_addr = 1;
                } 
                i++;
        }

        if (!got_addr) {
                strcpy(v6_local_host, "localhost");
        }


	/* initialize sockaddr's */
	sin1.sin6_family = AF_INET6;
	sin1.sin6_port = htons(PORT);

	if (!(inet_pton (AF_INET6, v6_local_host, &sin1.sin6_addr))) {
		tst_brkm(TBROK, cleanup, "invalid hostname: %s",
                        v6_local_host);
                return;
	}
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

start_server()
{
	int	sfd;

	sfd = socket(PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP);
	if (sfd < 0) {
		tst_brkm(TBROK, cleanup, "server socket failed: %s",
			strerror(errno));
		return -1;
	}
	if (bind(sfd, (struct sockaddr *) &server_addr, sizeof(sin1)) < 0) {
		tst_brkm(TBROK, cleanup, "server bind failed: %s",
			strerror(errno));
		return -1;
	}
	if (listen(sfd, 10) < 0) {
		tst_brkm(TBROK, cleanup, "server listen failed: %s",
			strerror(errno));
		return -1;
	}

}
