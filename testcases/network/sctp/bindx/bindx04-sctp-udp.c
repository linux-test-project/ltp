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
 * Test Name: bindx04-sctp-udp
 *
 * Test Description:
 *      test 1:
 *      Check if mutiple bindxes succeed. 
 *
 *      test 2:
 *      Check if a bindx call succeeds after a bind call. 
 *      
 *      test 3:
 *      Check if a bindx call succeeds if it is passed in different ports.
 *
 *      test 4: 
 *      Check if multiple bindxes with the same parameter failes. 
 *      Expect a failure. 
 *      
 *      
 * ALGORITHM
 *      test 1:
 *      Issue a bindx with valid parameters after a successful bindx call. 
 *
 *      test 2:
 *      Issue a bindx with valid parameters after a successful bind call. 
 *      
 *      test 3:
 *      Pass two addresses with port 10001 and 10002 to bindx. 
 *
 *      test 4: 
 *      Call bindx twice with the same address, socket and flag. 
 *      
 *
 * Usage:  <for command-line>
 *  bindx04-sctp-udp [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"

char *TCID="bindx04-sctp-udp";		/* Test program identifier.    */
int testno;
int addrcnt = 1;
int	s;	/* socket descriptor */
int port1 = 10001, port2 = 10002;
struct sockaddr_storage *sin1, *addrs;

void setup(void), setup0(void), setup1(void), 
	setup2(void), setup3(void), setup4(void),
	cleanup(void), cleanup1(void),
	cleanup2(void);

struct test_case_t {		/* test case structure */
	int	domain;		/* PF_INET, PF_INET6, ... */
	int	type;		/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;		/* protocol number */
	struct	sockaddr_storage **sockaddr;	
				/* socket address buffer */
	int	*addrcnt;	/* bindx's 3rd argument */
	int	flags;		/* bindx's 4th argument */
	int	retval;		/* syscall return value */
	int	experrno;	/* expected errno */
	void	(*setup)(void);
	void	(*cleanup)(void);
	char	*desc;
	
} tdat[] = {
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin1,
		&addrcnt, SCTP_BINDX_ADD_ADDR, 0, 0, setup1, cleanup2,
		"multiple bindx" }, 
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin1,
		&addrcnt, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup1,
		"bind/bindx" }, 
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin1,
		&addrcnt, SCTP_BINDX_ADD_ADDR, 0, 0, setup3, cleanup1,
		"bindxes with different ports" },  
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin1,
		&addrcnt, SCTP_BINDX_REM_ADDR, -1, EADDRINUSE, setup4, cleanup1,
		"bindxes with same address and same socket" },  
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
		
			TEST(bindx(s, *(tdat[testno].sockaddr),
				*(tdat[testno].addrcnt), tdat[testno].flags));
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
	if (NULL != sin1) {
		free (sin1);
		sin1 = NULL;
	}	
}

void
cleanup2(void)
{
	(void) close(s);
	if (NULL != sin1) {
		free (sin1);
		sin1 = NULL;
	}	
	if (NULL != addrs) {
		free (addrs);
		addrs = NULL;
	}
}

/* 
 *  Multiple bind test case. 
 *  Create a socket and bindx an address first. 
 */

void 
setup1(void)
{
	int count, i, got_first_addr, got_second_addr;
	char first_local_addr[64], second_local_addr[64];
	char v4_dev_name[16];
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

	/* get a list of ip v4 address. */
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

	/* search for another different v4 address. */
	while (i < count && !got_second_addr)
	{
		/*printf("if_name: %s dev: %s has_v4: %i\n", 
			local_addrs[i].if_name, v4_dev_name, 
			local_addrs[i].has_v4); */
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
	addrs = NULL;
	tmp_addrs = append_addr(first_local_addr, addrs, &addrcnt, port1);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, "unable to add address (%s) ",
			first_local_addr, "test %d", testno);
		tst_exit();
	} 
		
	addrs = tmp_addrs;
	if (bindx(s, addrs, addrcnt, SCTP_BINDX_ADD_ADDR)!=0) {
		tst_brkm(TBROK, cleanup, 
			"addr(%s) specified is invalid",
			first_local_addr);
		tst_exit();

	}	

	addrcnt = 0; 
	sin1= NULL;
	tmp_addrs = append_addr(second_local_addr, sin1, &addrcnt, port1);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, "unable to add address (%s) ",
			second_local_addr, "test %d", testno);
		tst_exit();
	} 
	sin1 = tmp_addrs;
}

/* 
 *  Create a socket and bind an address first. 
 */

void 
setup2(void)
{
	int count, i, got_first_addr, got_second_addr;
	char first_local_addr[64], second_local_addr[64];
	char v4_dev_name[16];
	struct hostent *hst;
	struct sockaddr_in *v4;
	struct sockaddr first_addr;
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
		/*printf("if_name: %s dev: %s has_v4: %i\n", 
			local_addrs[i].if_name, v4_dev_name, 
			local_addrs[i].has_v4); */
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

	v4 = (struct sockaddr_in *) &first_addr;

	la_raw = &v4->sin_addr;
	v4->sin_port = ntohs(port1);
	v4->sin_family = AF_INET;
	memcpy(la_raw, hst->h_addr_list[0], hst->h_length);

	if (bind(s, &first_addr, sizeof(first_addr))!=0) {
		tst_brkm(TBROK, cleanup, 
			"Cannot bind: %s",
			first_local_addr);
		tst_exit();

	}	

	addrcnt = 0; 
	addrs = NULL;
	tmp_addrs = append_addr(second_local_addr, sin1, &addrcnt, port1);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, "unable to add address (%s) ",
			second_local_addr, "test %d", testno);
		tst_exit();
	} 
	sin1 = tmp_addrs;
}
	

/* 
 *  Create a socket and bind two addresses with different port. 
 */
void 
setup3(void)
{
	int count, i, got_first_addr, got_second_addr;
	char first_local_addr[64], second_local_addr[64];
	char v4_dev_name[16];
	struct sockaddr_storage * tmp_addrs;

	local_addr_t    local_addrs[10];
	void *la_raw;  /* This is the addr part of local_addr. */
	int la_len = 0, family;

	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket create failed "
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
		tst_brkm(TBROK, cleanup, "unable to get second addr "
			"test %d", testno);
		tst_exit();
	}

	addrcnt = 0; 
	sin1 = NULL;
	tmp_addrs = append_addr(first_local_addr, sin1, &addrcnt, port1);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, "unable to add address (%s) ",
			first_local_addr, "test %d", testno);
		tst_exit();
	} 
	sin1 = tmp_addrs;
	tmp_addrs = append_addr(second_local_addr, sin1, &addrcnt, port2);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup1, "unable to add address (%s) ",
			second_local_addr, "test %d", testno);
		tst_exit();
	} 
	sin1 = tmp_addrs;


} /* setup3 */

/* 
 *  Create a socket and bindx an address first. 
 */

void 
setup4(void)
{
	int count, i, got_first_addr;
	char first_local_addr[64];
	char v4_dev_name[16];
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

	/* IPv4 addr over an IPv6 socket */
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
	

        if (!got_first_addr) {
                strcpy(first_local_addr, "localhost");
        }

	addrcnt = 0; 
	sin1 = NULL;
	tmp_addrs = append_addr(first_local_addr, sin1, &addrcnt, port1);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, 
			"addr(%s) specified is invalid",
			first_local_addr);
		tst_exit();
	} 
	if (bindx(s, tmp_addrs, addrcnt, SCTP_BINDX_ADD_ADDR)!=0) {
		tst_brkm(TBROK, cleanup1, 
			"bindx failed (%s) with errno: %i",
			first_local_addr, errno);
		tst_exit();

	}	
} /* setup4 */

