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
 * Test Name: bind03-sctp-udp
 *
 * Test Description:
 *  Bind API combination test cases. 
 * 
 *  Test 1: 
 *   Expect EADDRINUSE when multiple bind() are issues with exact same 
 *   parameters.
 *
 *  Test 2: 
 *   Expect failure when a bindx is called after a bind and both of the  
 *   calls are to bind some address to the same socket. 
 *
 * Usage:  <for command-line>
 *  bind03-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
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

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <netdb.h>

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"

char *TCID="bind03-sctp-udp";		/* Test program identifier.    */
int testno;
int	s;	/* socket descriptor */
int port1 = 10001;
struct sockaddr_in sin1;

void setup1(void), setup2(void), cleanup(void), cleanup1(void);

struct test_case_t {		/* test case structure */
	int	domain;		/* PF_INET, PF_INET6, ... */
	int	type;		/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;		/* protocol number (usually 0 = default) */
	struct	sockaddr	*sockaddr;	
				/* socket address buffer */
	int	addrlen;	/* bind's 3rd argument */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char	*desc;
	
} tdat[] = {
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr *)&sin1,
		sizeof(sin1), -1, EADDRINUSE, setup1, cleanup1,
		"multiple bind" }, 
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr *)&sin1,
		sizeof(sin1), -1, EINVAL, setup2, cleanup1,
		"bindx/bind" }, 
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
		
			TEST(bind(s, tdat[testno].sockaddr,
				tdat[testno].addrlen));
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
cleanup1(void)
{
	(void) close(s);
	
}

/* 
 *  Multiple bind test case. 
 *  Create a socket and bind an address first. 
 */

void 
setup1(void)
{
	int count, i, got_first_addr, got_second_addr;
	char first_local_addr[64], second_local_addr[64];
	char v4_dev_name[16];
	struct hostent *hst;
	struct sockaddr_in *v4;

	local_addr_t    local_addrs[10];
	void *la_raw;  /* This is the addr part of local_addr. */
	int la_len = 0, family;

	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for bindx "
			"test %d: %s", testno, strerror(errno));
		tst_exit();
	}

	/* get a list of IPv4 addresses */
	get_ip_addresses(local_addrs, &count);

	i = 0;
	got_first_addr = 0;	
	while (i < count && !got_first_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		     && local_addrs[i].has_v4) {
			strcpy(first_local_addr, local_addrs[i].v4_addr);
			got_first_addr = 1;
			strcpy(v4_dev_name, local_addrs[i].if_name);
		} 
		i++;
	} 
	
	i = 0;
	got_second_addr = 0;	
	while (i < count && !got_second_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		    && (strcmp(local_addrs[i].if_name, v4_dev_name))
		    && local_addrs[i].has_v4) {
			strcpy(second_local_addr, local_addrs[i].v4_addr);
			got_second_addr = 1;
		}
		i++;
	}

	if (!got_second_addr) {
		tst_brkm(TBROK, cleanup, "did not get second address"
			"test %d", testno);
		tst_exit();
	}

	hst = gethostbyname(first_local_addr);

	if (hst == NULL || hst->h_length < 1) {
		tst_brkm(TBROK, cleanup, "gethostbyname of %s failed ",
			first_local_addr, "test %d", testno);
		tst_exit();
	}

	v4 = (struct sockaddr_in *) &sin1;

	la_raw = &v4->sin_addr;
	v4->sin_port = ntohs(port1);
	v4->sin_family = AF_INET;
	memcpy(la_raw, hst->h_addr_list[0], hst->h_length);

	if (bind(s, (struct sockaddr *) &sin1, sizeof(sin1))!=0) {
		tst_brkm(TBROK, cleanup, 
			"Cannot bind: %s",
			first_local_addr);
		tst_exit();

	}	
}

/* 
 *  Create a socket and bind an address first using bindx. 
 */

void 
setup2(void)
{
	int count, i, got_first_addr, got_second_addr, addrcnt;
	char first_local_addr[64], second_local_addr[64];
	char v4_dev_name[16];
	struct hostent *hst;
	struct sockaddr_in *v4;
	struct sockaddr_storage *tmp_addrs;
	local_addr_t    local_addrs[10];
	void *la_raw;  /* This is the addr part of local_addr. */
	int la_len = 0, family;

	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for bindx "
			"test %d: %s", testno, strerror(errno));
		tst_exit();
	}


	get_ip_addresses(local_addrs, &count);

	i = 0;
	got_first_addr = 0;	
	while (i < count && !got_first_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		     && local_addrs[i].has_v4) {
			strcpy(first_local_addr, local_addrs[i].v4_addr);
			got_first_addr = 1;
			strcpy(v4_dev_name, local_addrs[i].if_name);
		} 
		i++;
	} 
	
	i = 0;
	got_second_addr = 0;	
	while (i < count && !got_second_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		    && (strcmp(local_addrs[i].if_name, v4_dev_name))
		    && local_addrs[i].has_v4) {
			strcpy(second_local_addr, local_addrs[i].v4_addr);
			got_second_addr = 1;
		}
		i++;
	}

	if (!got_second_addr) {
		tst_brkm(TBROK, cleanup, "did not get second address"
			"test %d", testno);
		tst_exit();
	}

	addrcnt = 0; 
	
	tmp_addrs = append_addr(first_local_addr, NULL, &addrcnt, port1);

	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, "unable to add address (%s) %s",
			first_local_addr, "test %d", testno);
		tst_exit();
	} 


	if (bindx(s, tmp_addrs, addrcnt, SCTP_BINDX_ADD_ADDR)!=0) {
		tst_brkm(TBROK, cleanup1, 
			"bindx failed: %s with errno: %i",
			first_local_addr, errno);
		tst_exit();

	}	
	
	hst = gethostbyname(second_local_addr);

	if (hst == NULL || hst->h_length < 1) {
		tst_brkm(TBROK, cleanup, "gethostbyname of %s failed. %s",
			first_local_addr, "test %d", testno);
		tst_exit();
	}

	v4 = (struct sockaddr_in *) &sin1;

	la_raw = &v4->sin_addr;
	v4->sin_port = ntohs(port1);
	v4->sin_family = AF_INET;
	memcpy(la_raw, hst->h_addr_list[0], hst->h_length);

} /* setup2 */
