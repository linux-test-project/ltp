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
 * Test Name: bindx03-sctp-udp
 *
 * Test Description:
 *      test 1:
 *      IPv4 addr/IPv6 socket combo. Expect it to fail. 
 *
 *      test 2:
 *      IPv6 addr/IPv6 socket combo. Should succeed.
 *      
 *      test 3:
 *      IPv4 and IPv6 addresses over IPv4 socket. Should fail.
 *
 *      test 4:
 *      IPv4 and IPv6 addresses over IPv6 socket. Should succeed.
 *
 *      test 5:
 *      IPv6 mapped IPv4 addr over an IPv6 socket. Should succeed. 
 *      
 *      test 6:
 *      A SCTP_BINDX_ADD_ADDR flag test. Should succeed.
 *      
 *      test 7:
 *      A SCTP_BINDX_REM_ADDR flag test. Should succeed.
 *
 *      
 * ALGORITHM
 *      test 1:
 *      Create an IPv6 socket and bind an IPv4 address to it.
 *
 *      test 2:
 *      Create an IPv6 socket and bind an IPv6 address to it.
 *      
 *      test 3:
 *      Create a IPv4 socket and bind two addresses (IPv4 + IPv6) to it.
 *
 *      test 4:
 *      Create a IPv6 socket and bind two addresses (IPv4 + IPv6) to it.
 *
 *      test 5:
 *      Create IPv6 socket and bind a IPv6 mappped IPv4 addr to it.
 *      
 *      test 6:
 *      Bind an IPv4 address to a socket and issue a bindx later.  
 *      
 *      test 7:
 *      Bind two addresses to the same socket and issue a bindx with 
 *      SCTP_BINDX_REM_ADDR later.
 *
 * Usage:  <for command-line>
 *  bindx03-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
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
 *  Error logging is not supported.
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

char *TCID="bindx03-sctp-udp";		/* Test program identifier.    */
int testno;
int addrcnt = 1;
int	s;	/* socket descriptor */
int port = 10001;
struct sockaddr_storage *sin1, *sin2, *sin3, *sin4, *sin5, *sin6, *addrs;

void setup(void), setup0(void), setup1(void), 
	setup2(void), setup3(void), setup4(void),
	cleanup(void), cleanup0(void), cleanup1(void),
	cleanup3(void);

struct test_case_t {		/* test case structure */
	int	domain;		/* PF_INET, PF_INET6, ... */
	int	type;		/* SOCK_STREAM, SOCK_SEQPACKET... */
	int	proto;		/* protocol number (usually 0 = default) */
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
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin2,
		&addrcnt, SCTP_BINDX_ADD_ADDR, -1, 0, setup2, cleanup0,
		"IPv4 addr over an IPv6 socket" }, 
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin3,
                &addrcnt, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup0,
                "IPv6 addr over an IPv6 socket" }, 
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin1,
		&addrcnt, SCTP_BINDX_ADD_ADDR, -1, 0, setup1, cleanup1,
		"Combo of IPv6 addr and IPv4 addr over an IPv4 socket" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin1,
		&addrcnt, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup1,
		"Combo of IPv6 addr and IPv4 addr over an IPv6 socket" },
	{ PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin4,
		&addrcnt, SCTP_BINDX_ADD_ADDR, 0, 0, setup2, cleanup0,
		"IPv6 mapped IPv4 addr over an IPv6 socket" }, 
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin5,
		&addrcnt, SCTP_BINDX_ADD_ADDR, 0, 0, setup3, cleanup0,
		"SCTP_BINDX_ADD_ADDR" }, 
	{ PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP, 
		(struct sockaddr_storage **)&sin6,
		&addrcnt, SCTP_BINDX_REM_ADDR, 0, 0, setup4, cleanup3,
		"SCTP_BINDX_REM_ADDR" },  
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
setup(void)
{
	int count, i, got_addr;
	char v4_local_host[64], v6_local_host[64];
	char v4_dev_name[16], v6_dev_name[16], temp_str[32];
	FILE * infile;
	struct hostent *hst;
	struct sockaddr_in *v4;
	struct sockaddr_in6 *v6;
	struct sockaddr_storage *tmp_addrs, *addrs;

	local_addr_t    local_addrs[10];
	void *la_raw;  /* This is the addr part of local_addr. */
	int la_len = 0, family;

	
	TEST_PAUSE;	/* if -p option specified */

	/* initialize sockaddr's */

	/* IPv4 addr over an IPv6 socket */
	get_ip_addresses(local_addrs, &count);

	i = 0;
	got_addr = 0;	
	while (i < count && !got_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		     && local_addrs[i].has_v4) {
			strcpy(v4_local_host, local_addrs[i].v4_addr);
			got_addr = 1;
			strcpy(v4_dev_name, local_addrs[i].if_name);
		}  
		i++;
	}

	if (!got_addr) {
		strcpy(v4_local_host, "localhost");
		strcpy(v4_dev_name, "lo");
	}

	hst = gethostbyname(v4_local_host);

	if (hst == NULL || hst->h_length < 1) {
		tst_brkm(TBROK, cleanup, 
			"bindx gethostbynane of %s failed", v4_local_host);
		tst_exit();
	}

	sin2 = (struct sockaddr_storage *) 
		malloc(sizeof (struct sockaddr_storage));
	if (NULL == sin2) {
		tst_brkm(TBROK, cleanup, "bindx no memory");
		tst_exit();
	}

	v4 = (struct sockaddr_in *) sin2;

	v4->sin_family = AF_INET;
	/* must be a unused port. */
	v4->sin_port = ntohs(port); 
	memcpy(&v4->sin_addr, hst->h_addr_list[0], hst->h_length);

	/* IPv6 addr over an IPv6 socket */
	i = 0;
	got_addr = 0;	
	while (i < count && !got_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		     && local_addrs[i].has_v6) {
			strcpy(v6_local_host, local_addrs[i].v6_addr);	
			got_addr = 1;
			strcpy(v6_dev_name, local_addrs[i].if_name);
		} 
		i++;
	}

	if (!got_addr) {
		strcpy(v6_local_host, "localhost");
		strcpy(v6_dev_name, "lo");
	}

	sin3 = (struct sockaddr_storage *) 
		malloc(sizeof (struct sockaddr_storage));
	if (NULL == sin3) {
		tst_brkm(TBROK, cleanup, "bindx no memory");
		tst_exit();
	}
	v6 = (struct sockaddr_in6 *) sin3;
	v6->sin6_family = AF_INET6;
	v6->sin6_port = ntohs(port); 

	if (!(inet_pton (AF_INET6, v6_local_host, &v6->sin6_addr))) {
        	printf (" error");
	}

	/* Combo of IPv4 addr and IPv6 addr. */

	if (!strcmp(v4_local_host, "localhost")){
		tst_brkm(TBROK, cleanup, 
		"bindx expecting two different IP addresses for a combo test");
		tst_exit();
	}

	i = 0;
	got_addr = 0;
	if (strcmp(local_addrs[0].if_name, v4_dev_name)) {
		got_addr = 1;
	}
	
	while (i < count && !got_addr)
	{
		if ((strcmp(local_addrs[i].if_name, "lo"))
		     && (strcmp(local_addrs[i].if_name, v4_dev_name))
		     && local_addrs[i].has_v6) {
			strcpy(v6_local_host, local_addrs[i].v6_addr);	
			got_addr = 1;
			strcpy(v6_dev_name, local_addrs[i].if_name);
		} 
		i++;
	}
	addrcnt = 0; 
	tmp_addrs = append_addr(v4_local_host, sin1, &addrcnt, port);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, 
			"addr(%s) specified is invalid",
			v4_local_host);
		tst_exit();
	} 
	sin1 = tmp_addrs;

	tmp_addrs = append_addr(v6_local_host, sin1, &addrcnt, port);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup1, "addr (%s) specified is invalid",
			v6_local_host);
		tst_exit();
	} 
	sin1 = tmp_addrs;

	/* IPv6 mapped IPv4 addr over an IPv6 socket */

	addrcnt = 1;
	sprintf(temp_str, "::ffff:%s", v4_local_host);
	hst = gethostbyname2(temp_str, AF_INET6);

	if (hst == NULL || hst->h_length < 1) {
		tst_brkm(TBROK, cleanup, 
			"bindx gethostbyname of %s failed", temp_str);
		tst_exit();
	}

	sin4 = (struct sockaddr_storage *) 
		malloc(sizeof (struct sockaddr_storage));
	if (NULL == sin4) {
		tst_brkm(TBROK, cleanup, "bindx no memory");
		tst_exit();
	}
	v6 = (struct sockaddr_in6 *) sin4;

	v6->sin6_family = AF_INET6;
	/* must be a unused port. */
	v6->sin6_port = ntohs(port);
	memcpy(&v6->sin6_addr, hst->h_addr_list[0], hst->h_length);

}


void 
setup0(void)
{
	s = socket(tdat[testno].domain, tdat[testno].type, tdat[testno].proto);
	if (s < 0) {
		tst_brkm(TBROK, cleanup, "socket setup failed for bindx "
			"test %d: %s", testno, strerror(errno));
		tst_exit();
	}
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
		tst_exit();
	}
}


/* 
 *  Create a socket and bind an address first. 
 */

void 
setup3(void)
{
	int count, i, got_first_addr, got_second_addr;
	char first_local_addr[64], second_local_addr[64];
	char v4_dev_name[16], v6_dev_name[16];
	FILE * infile;
	struct hostent *hst;
	struct sockaddr_in *v4;
	struct sockaddr_in6 *v6;
	struct sockaddr first_addr;

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
	
	//printf("Dev: %s\n", v4_dev_name);
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
			//printf("get here\n");
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
	v4->sin_port = ntohs(port);
	v4->sin_family = AF_INET;
	memcpy(la_raw, hst->h_addr_list[0], hst->h_length);

	bind(s, &first_addr, sizeof(first_addr));

	hst = gethostbyname(second_local_addr);

	if (hst == NULL || hst->h_length < 1) {
		tst_brkm(TBROK, cleanup, "gethostbyname of %s failed",
			second_local_addr, "test %d", testno);
		tst_exit();
	}

	sin5 = (struct sockaddr_storage *) 
		malloc(sizeof (struct sockaddr_storage));
	if (NULL == sin5) {
		tst_brkm(TBROK, cleanup, "bindx no memory");
		tst_exit();
	}
	v4 = (struct sockaddr_in *) sin5;

	la_raw = &v4->sin_addr;
	v4->sin_port = ntohs(port);
	v4->sin_family = AF_INET;
	memcpy(la_raw, hst->h_addr_list[0], hst->h_length);
	
}

/* 
 *  Create a socket and bind two addresses to it. 
 */
void 
setup4(void)
{
	int count, i, got_first_addr, got_second_addr;
	char first_local_addr[64], second_local_addr[64];
	char v4_dev_name[16], v6_dev_name[16];
	FILE * infile;
	struct hostent *hst;
	struct sockaddr_in *v4;
	struct sockaddr_in6 *v6;
	struct sockaddr_in first_addr;
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
	//printf("addrcnt: %i\n", addrcnt);
	tmp_addrs = append_addr(first_local_addr, addrs, &addrcnt, port);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, "unable to add address (%s) ",
			first_local_addr, "test %d", testno);
		tst_exit();
	} 
	//printf("addrcnt: %i\n", addrcnt);
	addrs = tmp_addrs;

	tmp_addrs = append_addr(second_local_addr, addrs, &addrcnt, port);
	if (NULL == tmp_addrs) {
		tst_brkm(TBROK, cleanup, "unable to add address (%s) ",
			second_local_addr, "test %d", testno);
		tst_exit();
	} 
	addrs = tmp_addrs;

	bindx (s, addrs, addrcnt, SCTP_BINDX_ADD_ADDR);

	/* save the first address */
	addrcnt = 1;
	hst = gethostbyname(first_local_addr);

	if (hst == NULL || hst->h_length < 1) {
		tst_brkm(TBROK, cleanup, "gethostbyname (%s) failed ", 
			first_local_addr, "test %d", testno);
		tst_exit();
	}

	sin6 = (struct sockaddr_storage *) 
		malloc(sizeof (struct sockaddr_storage));
	if (NULL == sin6) {
		tst_brkm(TBROK, cleanup, "bindx no memory");
		tst_exit();
	}
	
	v4 = (struct sockaddr_in *) sin6;
	v4->sin_family = AF_INET;
	/* must be a unused port. */
	v4->sin_port = ntohs(port);
	memcpy(&v4->sin_addr, hst->h_addr_list[0], hst->h_length);

}

void
cleanup0(void)
{
	(void) close(s);
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
cleanup3(void)
{
	(void) close(s);
	if (NULL != addrs) {
		free (addrs);
		addrs = NULL;
	}
}
