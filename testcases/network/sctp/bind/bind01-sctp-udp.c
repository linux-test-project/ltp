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
 * Test Name: bind01-sctp-udp
 *
 * Test Description:
 *  Test 1:
 *   Expect EFAULT when passed in an invalid sockaddr. 
 *
 *  Test 2:
 *   Expect EINVAL when passed in an invalid sockaddr length. 
 *
 *  Test 3: 
 *   Expect ENOTSOCK when passed in an invalid socket descriptor.
 *
 *  Test 4: 
 *   Expect EADDRNOTAVAIL when passed in an invalid host name. 
 * 
 *  Test 5:
 *   Expect bind to succeed when asked to bind to an INADDR_ANY address and 
 *   an non-zero port. 
 *
 *  Test 6:
 *   Expect bind to succeed when asked to bind to an INADDR_ANY address and 
 *   zero port. 
 *
 *  Test 7:
 *   Expect bind to succeed when asked to bind to local address and
 *   zero port.
 * 
 *  Test 8: 
 *   Expect bind to succeed when asked to bind to an INADDR_ANY address and
 *   an non zero port.
 *
 * Usage:  <for command-line>
 *  bind01-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      02/2002 Adapted for SCTP by Robbie Williamson
 *      04/2002 Added new test cases by Mingqin Liu
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
#include <sys/un.h>

#include <netinet/in.h>
#include <netdb.h>

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"

char *TCID="bind01-sctp-udp";		/* Test program identifier.    */
int testno;

int	s;	/* socket descriptor */

struct sockaddr_in sin1, sin2, sin3, sin4, sin5;

void setup(void), setup0(void), setup1(void), setup2(void), setup3(void),
	cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int	domain;	/* PF_INET, PF_INET6, ... */
	int	type;	/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;	/* protocol number (usually 0 = default) */
	struct sockaddr *sockaddr;	/* socket address buffer */
	int	salen;	/* bind's 3rd argument */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char *desc;
} tdat[] = {
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)-1,
		sizeof(struct sockaddr_in), -1, EFAULT, setup0, cleanup0,
		"invalid sockaddr for UDP style SCTP" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin1,
		3, -1, EINVAL, setup0, cleanup0,
		"invalid salen for UDP style SCTP" },
	{ 0, 0, IPPROTO_SCTP, (struct sockaddr *)&sin1,
		sizeof(sin1), -1, ENOTSOCK, setup1, cleanup1,
		"invalid socket for UDP style SCTP" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin3,
		sizeof(sin3), -1, EADDRNOTAVAIL, setup0, cleanup0,
		"non-local address for UDP style SCTP" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin1,
		sizeof(sin1), 0, 0, setup0, cleanup0,
		"INADDR_ANY/non zero port for UDP style SCTP" },
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin2,
		sizeof(sin2), 0, 0, setup0, cleanup0,
		"INADDR_ANY/zero port for UDP style SCTP" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin4,
               sizeof(sin4), 0, 0, setup0, cleanup0,
                "local addr/zero port for IPv4 UDP style SCTP socket" },
        { PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin5,
                sizeof(sin5), 0, 0, setup0, cleanup0,
                "a regular bind" },
};

int TST_TOTAL=sizeof(tdat)/sizeof(tdat[0]); /* Total number of test cases. */

extern int Tst_count;

int
main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *) NULL) {
		tst_brkm(TBROK, 0, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;

		for (testno=0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();

			TEST(bind(s, tdat[testno].sockaddr,tdat[testno].salen));
			if (TEST_RETURN > 0) {
				TEST_RETURN = 0;
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
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
				tst_resm(TPASS, "%s successful",
					tdat[testno].desc);
			}
			tdat[testno].cleanup();
		} /* for (testno=0; testno < TST_TOTAL; ++testno) */

	} /* for (lc = 0; TEST_LOOPING(lc); ++lc)  */
	cleanup();
}	/* End main */

void
setup(void)
{
        int count, i, got_addr;
        char local_host[64];
        struct hostent *hst;
        local_addr_t    local_addrs[10];
        void *la_raw;  /* This is the addr part of local_addr. */

	TEST_PAUSE;	/* if -p option specified */

	/* initialize sockaddr's */
	sin1.sin_family = AF_INET;
		/* this port must be unused! */
	sin1.sin_port = htons((getpid() % 32768) +10000);
	sin1.sin_addr.s_addr = INADDR_ANY;

	sin2.sin_family = AF_INET;
	sin2.sin_port = 0;
	sin2.sin_addr.s_addr = INADDR_ANY;

	sin3.sin_family = AF_INET;
	sin3.sin_port = 0;
		/* assumes 10.255.254.253 is not a local interface address! */
	sin3.sin_addr.s_addr = htonl(0x0AFFFEFD);

        /* local IP addr/0 port. */
        
        get_ip_addresses(local_addrs, &count);

        i = 0;
        got_addr = 0;   
        while (i < count && !got_addr)
        {
                if ((strcmp(local_addrs[i].if_name, "lo"))
                     && local_addrs[i].has_v4) {
                        strcpy(local_host, local_addrs[i].v4_addr);
                        got_addr = 1;
                }  
                i++;
        }

        if (!got_addr) {
                strcpy(local_host, "localhost");
        }

        //printf ("i= %i, local_host: %s\n",i, local_host);

        hst = gethostbyname(local_host);

        if (hst == NULL || hst->h_length < 1) {
                TEST_ERROR_LOG(TEST_ERRNO);
                exit(-1);
        }

        la_raw = &sin4.sin_addr;
        sin4.sin_port = 0;
        sin4.sin_family = AF_INET;

        memcpy(la_raw, hst->h_addr_list[0], hst->h_length);

        sin5.sin_family = AF_INET;
        /* must be a unused port. */
        sin5.sin_port = htons((getpid() % 32768) +10001);
        memcpy(&(sin5.sin_addr), hst->h_addr_list[0], hst->h_length);

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
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for bind "
			"test %d: %s", testno, strerror(errno));
	}
}

void
cleanup0(void)
{
	(void) close(s);
}

void
setup1(void)
{
	s = 0;	/* setup for the "not a socket" case */
}

void
cleanup1(void)
{
	s = -1;
}
