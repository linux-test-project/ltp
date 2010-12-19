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
 * Test Name: asapi_05
 *
 * Test Description:
 *  These tests are for the "Advanced Sockets API" (RFC 3542)
 *  Verify that in6 and sockaddr fields are present.
 *
 * Usage:  <for command-line>
 *  asapi_05
 *
 * HISTORY
 *	04/2005 written by David L Stevens
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/wait.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#include "test.h"
#include "usctest.h"
#include "runcc.h"

char *TCID="asapi_05";		/* Test program identifier.    */

void setup(void);
void cleanup(void);

void icmp6_et(void);
void icmp6_ft(void);

int
main(int argc, char *argv[])
{
	int	lc;
	char	*msg;

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		icmp6_et();
		icmp6_ft();
	}

	cleanup();

	tst_exit();
}

enum ttype { EXISTS, ALIAS, VALUE, DEFINED };

struct etent {
	char	*et_tname;		/* test name */
	int	et_type;		/* test type */
	char	*et_incl;		/* include file list */
	char	*et_struct;		/* structure name */
	char	*et_field;		/* field name */
	char	*et_offset;		/* field offset */
	union {
		char	*fu_value;	/* field size or value */
		char	*fu_dname;	/* #define name */
	} ftun;
#define et_value	ftun.fu_value
#define et_dname	ftun.fu_dname
} etab[] = {
/* existence checks, RFC 3542 section 3 */
	{ "icmp6_filter icmp6_filt", EXISTS, ICMP6_H, "icmp6_filter",
		"icmp6_filt", "0", {"32"} },
	{ "icmp6_filter icmp6_filt[0]", EXISTS, ICMP6_H, "icmp6_filter",
		"icmp6_filt[0]", "0", {"4"} },
	{ "ICMP6_FILTER_WILLPASS", DEFINED, ICMP6_H, "ICMP6_FILTER_WILLPASS",
		NULL, NULL, {0} },
	{ "ICMP6_FILTER_WILLBLOCK", DEFINED, ICMP6_H, "ICMP6_FILTER_WILLBLOCK",
		NULL, NULL, {0} },
	{ "ICMP6_FILTER_SETPASS", DEFINED, ICMP6_H, "ICMP6_FILTER_SETPASS",
		NULL, NULL, {0} },
	{ "ICMP6_FILTER_SETBLOCK", DEFINED, ICMP6_H, "ICMP6_FILTER_SETBLOCK",
		NULL, NULL, {0} },
	{ "ICMP6_FILTER_SETPASSALL", DEFINED, ICMP6_H,"ICMP6_FILTER_SETPASSALL",
		NULL, NULL, {0} },
	{ "ICMP6_FILTER_SETBLOCKALL",DEFINED,ICMP6_H,"ICMP6_FILTER_SETBLOCKALL",
		NULL, NULL, {0} },
	{ "ICMP6_FILTER", DEFINED, ICMP6_H, "ICMP6_FILTER", NULL, NULL, {0} },
/* existence checks, RFC 3542 section 4 */
/* socket options */
	{ "IPV6_RECVPKTINFO", VALUE, IN_H, "IPV6_RECVPKTINFO", NULL, NULL,
		{"IPV6_RECVPKTINFO"} },
	{ "IPV6_RECVHOPLIMIT", VALUE, IN_H, "IPV6_RECVHOPLIMIT", NULL, NULL,
		{"IPV6_RECVHOPLIMIT"} },
	{ "IPV6_RECVRTHDR", VALUE, IN_H, "IPV6_RECVRTHDR", NULL, NULL,
		{"IPV6_RECVRTHDR"} },
	{ "IPV6_RECVHOPOPTS", VALUE, IN_H, "IPV6_RECVHOPOPTS", NULL, NULL,
		{"IPV6_RECVHOPOPTS"} },
	{ "IPV6_RECVDSTOPTS", VALUE, IN_H, "IPV6_RECVDSTOPTS", NULL, NULL,
		{"IPV6_RECVDSTOPTS"} },
	{ "IPV6_RECVTCLASS", VALUE, IN_H, "IPV6_RECVTCLASS", NULL, NULL,
		{"IPV6_RECVTCLASS"} },
/* cmsg types */
	{ "IPV6_PKTINFO", DEFINED, IN_H, "IPV6_PKTINFO", NULL, NULL, {0} },
	{ "IPV6_HOPLIMIT", DEFINED, IN_H, "IPV6_HOPLIMIT", NULL, NULL, {0} },
	{ "IPV6_NEXTHOP", DEFINED, IN_H, "IPV6_NEXTHOP", NULL, NULL, {0} },
	{ "IPV6_RTHDR", DEFINED, IN_H, "IPV6_RTHDR", NULL, NULL, {0} },
	{ "IPV6_HOPOPTS", DEFINED, IN_H, "IPV6_HOPOPTS", NULL, NULL, {0} },
	{ "IPV6_DSTOPTS", DEFINED, IN_H, "IPV6_DSTOPTS", NULL, NULL, {0} },
	{ "IPV6_RTHDRDSTOPTS", DEFINED, IN_H, "IPV6_RTHDRDSTOPTS", NULL, NULL, {0} },
	{ "IPV6_TCLASS", DEFINED, IN_H, "IPV6_TCLASS", NULL, NULL, {0} },
};

#define ETCOUNT	(sizeof(etab)/sizeof(etab[0]))

/*  existence tests */
void
icmp6_et(void)
{
	int	i;

	for (i=0; i<ETCOUNT; ++i) {
		switch (etab[i].et_type) {
		case EXISTS:
			structcheck(etab[i].et_tname, etab[i].et_incl,
				etab[i].et_struct, etab[i].et_field,
				etab[i].et_offset, etab[i].et_value);
			break;
		case ALIAS:
			aliascheck(etab[i].et_tname, etab[i].et_incl,
				etab[i].et_struct, etab[i].et_field,
				etab[i].et_dname);
			break;
		case VALUE:
			valuecheck(etab[i].et_tname, etab[i].et_incl,
				etab[i].et_struct, etab[i].et_dname);
			break;
		case DEFINED:
			funccheck(etab[i].et_tname, etab[i].et_incl,
				etab[i].et_struct);
			break;
		default:
			tst_resm(TBROK, "invalid type %d",
				etab[i].et_type);
			break;
		}
	}
}

void
setup(void)
{
	TEST_PAUSE;	/* if -P option specified */
	tst_require_root(NULL);
}

void
cleanup(void)
{
	TEST_CLEANUP;

}

/*
 * This is for old, broken glibc-header icmp6_filter structure definitions.
 * If icmp6.h has struct icmp6_filter with field named "data" instead
 * of the standard "icmp_filt", uncomment this line.
 */
/*#define icmp_filt	data */

enum tt { T_WILLPASS, T_WILLBLOCK, T_SETPASS, T_SETBLOCK, T_SETPASSALL,
		T_SETBLOCKALL };
struct ftent {
	char		*ft_tname;	/* test name, for logging */
	unsigned char	ft_sndtype;	/* send type field */
	unsigned char	ft_flttype;	/* filter type field */
	enum tt		ft_test;	/* what macro to test */
	int		ft_expected;	/* packet should pass? */
} ftab[] = {
	{ "ICMP6_FILTER_SETPASS s 20 f 20", 20, 20,  T_SETPASS, 1 },
	{ "ICMP6_FILTER_SETPASS s 20 f 21", 20, 21, T_SETPASS, 0 },
	{ "ICMP6_FILTER_SETBLOCK s 20 f 20", 20, 20, T_SETBLOCK, 0 },
	{ "ICMP6_FILTER_SETBLOCK s 20 f 21", 20, 21, T_SETBLOCK, 1 },
	{ "ICMP6_FILTER_PASSALL s 20", 20, 0, T_SETPASSALL, 1 },
	{ "ICMP6_FILTER_PASSALL s 20", 21, 0, T_SETPASSALL, 1 },
	{ "ICMP6_FILTER_BLOCKALL s 20", 20, 0, T_SETBLOCKALL, 0 },
	{ "ICMP6_FILTER_BLOCKALL s 20", 21, 0, T_SETBLOCKALL, 0 },
	{ "ICMP6_FILTER_WILLBLOCK s 20 f 21", 20, 21, T_WILLBLOCK, 0 },
	{ "ICMP6_FILTER_WILLBLOCK s 20 f 20", 20, 20, T_WILLBLOCK, 1 },
	{ "ICMP6_FILTER_WILLPASS s 20 f 21", 20, 21, T_WILLPASS, 0 },
	{ "ICMP6_FILTER_WILLPASS s 22 f 22", 22, 22, T_WILLPASS, 1 },
};

#define FTCOUNT	(sizeof(ftab)/sizeof(ftab[0]))

int
ic6_send1(char *tname, unsigned char type)
{
	struct sockaddr_in6	sin6;
	struct icmp6_hdr	ic6;
	int			s;

	s = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (s == -1) {
		tst_resm(TBROK, "%s: socket failed", tname);
		return 1;
	}
	memset(&ic6, 0, sizeof(ic6));
	ic6.icmp6_type = type;
	ic6.icmp6_data32[0] = htonl(getpid());

	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = in6addr_loopback;
	if (sendto(s, &ic6, sizeof(ic6), 0, (struct sockaddr *)&sin6,
			sizeof(sin6)) == -1) {
		tst_resm(TBROK|TERRNO, "%s: sendto failed", tname);
		return 1;
	}
	return 0;
}

int
ic6_recv1(char *tname, int sall, int sf)
{
	fd_set	readfds, readfds_saved;
	struct timeval	tv;
	int	maxfd, nfds;
	int	gotall, gotone;
	int	cc;
	static	unsigned char rbuf[2048];

	tv.tv_sec = 0;
	tv.tv_usec = 250000;

	FD_ZERO(&readfds_saved);
	FD_SET(sall, &readfds_saved);
	FD_SET(sf, &readfds_saved);
	maxfd = MAX(sall, sf);

	memcpy(&readfds, &readfds_saved, sizeof(readfds));

	gotall = gotone = 0;
	/*
	 * Note: this relies on linux-specific behavior (select
	 * updating tv with time elapsed)
	 */
	while (!gotall || !gotone) {
		struct icmp6_hdr	*pic6 = (struct icmp6_hdr *)rbuf;

		nfds = select(maxfd+1, &readfds, 0, 0, &tv);
		if (nfds == 0)
			break;	/* timed out */
		if (nfds < 0) {
			if (errno == EINTR)
				continue;
			tst_resm(TBROK|TERRNO, "%s: select failed", tname);
		}
		if (FD_ISSET(sall, &readfds)) {
			cc = recv(sall, rbuf, sizeof(rbuf), 0);
			if (cc < 0) {
				tst_resm(TBROK|TERRNO,
				    "%s: recv(sall, ..) failed", tname);
				return -1;
			}
			/* if packet check succeeds... */
			if (htonl(pic6->icmp6_data32[0]) == getpid())
				gotall = 1;
		}
		if (FD_ISSET(sf, &readfds)) {
			cc = recv(sf, rbuf, sizeof(rbuf), 0);
			if (cc < 0) {
				tst_resm(TBROK|TERRNO,
				    "%s: recv(sf, ..) failed", tname);
				return -1;
			}
			/* if packet check succeeds... */
			if (htonl(pic6->icmp6_data32[0]) == getpid())
				gotone = 1;
		}
		memcpy(&readfds, &readfds_saved, sizeof(readfds));
	}
	if (!gotall) {
		tst_resm(TBROK, "%s: recv all timed out", tname);
		return -1;
	}
	if (gotone)
		return 1;
	return 0;
}

/* functional tests */
void
icmp6_ft(void)
{
	struct icmp6_filter	i6f;
	int sall, sf;
	int i;

	sall = socket(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (sall < 0) {
		tst_resm(TBROK|TERRNO,
			"icmp6_ft socket: can't create sall socket");
		return;
	}
	ICMP6_FILTER_SETPASSALL(&i6f);
	if (setsockopt(sall, IPPROTO_ICMPV6, ICMP6_FILTER, &i6f,
			sizeof(i6f)) < 0) {
		tst_resm(TBROK|TERRNO, "setsockopt pass all ICMP6_FILTER failed");
	}

	sf = socket(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (sf < 0) {
		tst_resm(TBROK|TERRNO, "icmp6_ft socket: can't create test socket");
		return;
	}

	int rv;

	for (i = 0; i < FTCOUNT; ++i) {

		rv = -1;

		switch (ftab[i].ft_test) {
		case T_SETPASS:
			ICMP6_FILTER_SETBLOCKALL(&i6f);
			ICMP6_FILTER_SETPASS(ftab[i].ft_flttype, &i6f);
			break;
		case T_SETPASSALL:
			ICMP6_FILTER_SETPASSALL(&i6f);
			break;
		case T_SETBLOCK:
			ICMP6_FILTER_SETPASSALL(&i6f);
			ICMP6_FILTER_SETBLOCK(ftab[i].ft_flttype, &i6f);
			break;
		case T_SETBLOCKALL:
			ICMP6_FILTER_SETBLOCKALL(&i6f);
			break;
		case T_WILLBLOCK:
			ICMP6_FILTER_SETPASSALL(&i6f);
			ICMP6_FILTER_SETBLOCK(ftab[i].ft_flttype, &i6f);
			rv = ICMP6_FILTER_WILLBLOCK(ftab[i].ft_sndtype, &i6f);
			break;
		case T_WILLPASS:
			ICMP6_FILTER_SETBLOCKALL(&i6f);
			ICMP6_FILTER_SETPASS(ftab[i].ft_flttype, &i6f);
			rv = ICMP6_FILTER_WILLPASS(ftab[i].ft_sndtype, &i6f);
			break;
		default:
			tst_resm(TBROK, "%s: unknown test type %d",
				ftab[i].ft_tname, ftab[i].ft_test);
			continue;
		}
		if (ftab[i].ft_test != T_WILLBLOCK &&
		    ftab[i].ft_test != T_WILLPASS) {
			if (setsockopt(sf, IPPROTO_ICMPV6, ICMP6_FILTER, &i6f,
					sizeof(i6f)) < 0) {
				tst_resm(TFAIL|TERRNO, "setsockopt ICMP6_FILTER");
				continue;
			}
			if (ic6_send1(ftab[i].ft_tname, ftab[i].ft_sndtype))
				continue;
			rv = ic6_recv1(ftab[i].ft_tname, sall, sf);
		} else
			rv = -1;

		if (rv < 0)
			continue;
		if (rv != ftab[i].ft_expected)
			tst_resm(TFAIL, "%s: rv %d != expected %d",
				ftab[i].ft_tname, rv, ftab[i].ft_expected);
		else
			tst_resm(TPASS, ftab[i].ft_tname);
	}
}

int TST_TOTAL = ETCOUNT;