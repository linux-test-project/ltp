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
 * Test Name: bindx02-sctp-udp
 *
 * Test Description:
 *  Test 1: 
 *   IN6ADDR_ANY_INIT/non-zero port combo. Should succeed. 
 *
 *  Test 2: 
 *   IN6ADDR_ANY_INIT/zero port combo. Should succeed.
 * 
 *  Test 3: 
 *   local IPv6 addr/zero port combo. Should succeed. 
 *
 *  Test 4: 
 *   A regular bindx to IPv6 socket. Should succeed. 
 * 
 * Usage:  <for command-line>
 *  bindx02-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	04/2002 Created by Mingqin Liu	
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <netinet/in.h>
#include <netinet/sctp.h>
#include <netinet/bindx.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"

char *TCID="bindx02-sctp-udp";		/* Test program identifier.    */
int testno;

int	s;	/* socket descriptor */

struct sockaddr_storage sin1, sin2, sin3, sin4;

void setup(void), setup0(void), setup1(void), setup2(void),
	cleanup(void), cleanup0(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int	domain;		/* PF_INET, PF_INET6, ... */
	int	type;		/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;		/* protocol number (usually 0 = default) */
	struct	sockaddr_storage *sockaddr;	
				/* socket address buffer */
	int	addrcnt;	/* bindx's 3rd argument */
	int	flags;		/* bindx's 4th argument */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char	*desc;
} tdat[] = {
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage *)&sin1,
		1, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup0,
		"IN6ADDR_ANY_INIT/non-zero port for UDP style SCTP" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage *)&sin2,
		1, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup0,
		"IN6ADDR_ANY_INIT/zero port for UDP style SCTP" }, 
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage *)&sin4,
                1, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup0,
                "local IPv6 addr/zero port for UDP style SCTP" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage *)&sin3,
		1, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup0,
		"a regular bindx of v6 addr" }, 
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

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;

		for (testno=0; testno < TST_TOTAL; ++testno) {
			tdat[testno].setup();
			TEST(bindx(s, tdat[testno].sockaddr,
				tdat[testno].addrcnt, tdat[testno].flags));
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
cleanup(void)
{
	TEST_CLEANUP;
	tst_exit();
}

void
setup(void)
{
	int count, i, got_addr;
	char local_host[64];
	struct hostent *hst;
	struct sockaddr_in *v4;
	struct sockaddr_in6 *v6;
	local_addr_t    local_addrs[10];
	void *la_raw;  /* This is the addr part of local_addr. */
	int la_len = 0, family;

	
	TEST_PAUSE;	/* if -p option specified */

	/* initialize sockaddr's */

	/* AF_INET6/IN6ADDR_ANY/non-zero port. */
	v6 = (struct sockaddr_in6 *) &sin1;
	v6->sin6_family = AF_INET6;
		/* this port must be unused! */
	v6->sin6_port = htons((getpid() % 32768) + 9999);
	v6->sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;

	/* AF_INET6/IN6ADDR_ANY/0 port */
	v6 = (struct sockaddr_in6 *) &sin2;
	v6->sin6_family = AF_INET6;
	v6->sin6_port = 0;
	v6->sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;

	/* local IP addr/0 port. */

	get_ip_addresses(local_addrs, &count);

	i = 0;
	got_addr = 0;	
	while (i < count && !got_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		     && local_addrs[i].has_v6) {
			strcpy(local_host, local_addrs[i].v6_addr);	
			got_addr = 1;
		} 
		i++;
	}

	if (!got_addr) {
		strcpy(local_host, "localhost");
	}

	hst = gethostbyname2(local_host, AF_INET6); 

	if (hst == NULL || hst->h_length < 1) {
		TEST_ERROR_LOG(TEST_ERRNO);
		exit(-1);
	}

	v6 = (struct sockaddr_in6 *) &sin3;
	v6->sin6_family = AF_INET6;
	v6->sin6_port = htons((getpid() % 32768) +10002);
	memcpy(&(v6->sin6_addr), hst->h_addr_list[0], hst->h_length);

	v6 = (struct sockaddr_in6 *) &sin4;
	v6->sin6_family = AF_INET6;
	v6->sin6_port = 0; 
	memcpy(&(v6->sin6_addr), hst->h_addr_list[0], hst->h_length);
}


void 
setup0(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for bindx "
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
setup2(void)
{
	setup();
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for bindx "
			"test %d: %s", testno, strerror(errno));
	}
}

void
cleanup1(void)
{
	s = -1;
}
