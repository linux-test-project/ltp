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
 * Test Name: bind02-sctp-udp
 *
 * Test Description:
 *  Test 1:
 *   IN6ADDR_ANY_INIT / an non-zero port to IPv6 socket.
 *  
 *  Test 2: 
 *   IN6ADDR_ANY/zero-port to IPv6 socket. 
 *
 *  Test 3: 
 *   Any local host / zero port to IPv6 socket. 
 *
 *  Test 4: 
 *   A regular bind to IPv6 socket. 
 *
 * Usage:  <for command-line>
 *  bind02-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
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
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"

char *TCID="bind02-sctp-udp";		/* Test program identifier.    */
int testno;

int	s;	/* socket descriptor */

struct sockaddr_in sin1, sin2;
struct sockaddr_in6 sin3, sin4, sin5, sin6;

void setup(void), setup1(void),
	cleanup(void), cleanup0(void);

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
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin3,
		sizeof(sin3), 0, 0, setup1, cleanup0,
		"IN6ADDR_ANY_INIT/non-zero port for UDP style SCTP" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin4,
		sizeof(sin4), 0, 0, setup1, cleanup0,
		"IN6ADDR_ANY_INIT/zero port for UDP style SCTP" }, 
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin6,
               sizeof(sin6), 0, 0, setup1, cleanup0,
                "local addr/zero port for IPv6 UDP style SCTP socket" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, (struct sockaddr *)&sin5,
		sizeof(sin5), 0, 0, setup1, cleanup0,
		"a regular bind of v6 addr" }, 
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
	local_addr_t    local_addrs[10];
	void *la_raw;  /* This is the addr part of local_addr. */

	TEST_PAUSE;	/* if -p option specified */

	/* initialize sockaddr's */

	/* AF_INET6/IN6ADDR_ANY/non-zero port. */
	sin3.sin6_family = AF_INET6;
		/* this port must be unused! */
	sin3.sin6_port = htons((getpid() % 32768) + 9999);
	sin3.sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;

	/* AF_INET6/IN6ADDR_ANY/0 port */
	sin4.sin6_family = AF_INET6;
	sin4.sin6_port = 0;
	sin4.sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;

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

	la_raw = &sin1.sin_addr;
	sin1.sin_port = 0;
	sin1.sin_family = AF_INET;

	memcpy(la_raw, hst->h_addr_list[0], hst->h_length);

	sin2.sin_family = AF_INET;
	/* must be a unused port. */
	sin2.sin_port = htons((getpid() % 32768) +10001);
	memcpy(&(sin2.sin_addr), hst->h_addr_list[0], hst->h_length);

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

	//printf ("i= %i, local_host: %s\n",i, local_host);

	hst = gethostbyname2(local_host, AF_INET6); 

	if (hst == NULL || hst->h_length < 1) {
		TEST_ERROR_LOG(TEST_ERRNO);
		exit(-1);
	}

	sin5.sin6_family = AF_INET6;
	sin5.sin6_port = htons((getpid() % 32768) +10002);
	memcpy(&sin5.sin6_addr, hst->h_addr_list[0], hst->h_length);

	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = 0;
	memcpy(&sin6.sin6_addr, hst->h_addr_list[0], hst->h_length);
}


void
cleanup0(void)
{
	(void) close(s);
}

void 
setup1(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for bind "
			"test %d: %s", testno, strerror(errno));
	}
}

