/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: David L Stevens
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
#include "safe_macros.h"

char *TCID = "asapi_02";

static void setup(void);

static void icmp6_ft(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc)
		icmp6_ft();

	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_require_root();
}

enum tt {
	T_WILLPASS,
	T_WILLBLOCK,
	T_SETPASS,
	T_SETBLOCK,
	T_SETPASSALL,
	T_SETBLOCKALL
};

static struct ftent {
	char *ft_tname;			/* test name, for logging */
	unsigned char ft_sndtype;	/* send type field */
	unsigned char ft_flttype;	/* filter type field */
	enum tt ft_test;		/* what macro to test */
	int ft_expected;		/* packet should pass? */
} ftab[] = {
	{"ICMP6_FILTER_SETPASS s 20 f 20", 20, 20, T_SETPASS, 1},
	{"ICMP6_FILTER_SETPASS s 20 f 21", 20, 21, T_SETPASS, 0},
	{"ICMP6_FILTER_SETBLOCK s 20 f 20", 20, 20, T_SETBLOCK, 0},
	{"ICMP6_FILTER_SETBLOCK s 20 f 21", 20, 21, T_SETBLOCK, 1},
	{"ICMP6_FILTER_PASSALL s 20", 20, 0, T_SETPASSALL, 1},
	{"ICMP6_FILTER_PASSALL s 20", 21, 0, T_SETPASSALL, 1},
	{"ICMP6_FILTER_BLOCKALL s 20", 20, 0, T_SETBLOCKALL, 0},
	{"ICMP6_FILTER_BLOCKALL s 20", 21, 0, T_SETBLOCKALL, 0},
	{"ICMP6_FILTER_WILLBLOCK s 20 f 21", 20, 21, T_WILLBLOCK, 0},
	{"ICMP6_FILTER_WILLBLOCK s 20 f 20", 20, 20, T_WILLBLOCK, 1},
	{"ICMP6_FILTER_WILLPASS s 20 f 21", 20, 21, T_WILLPASS, 0},
	{"ICMP6_FILTER_WILLPASS s 22 f 22", 22, 22, T_WILLPASS, 1},
};

#define FTCOUNT	ARRAY_SIZE(ftab)

static int ic6_send1(char *tname, unsigned char type)
{
	struct sockaddr_in6 sin6;
	struct icmp6_hdr ic6;
	int s;

	s = SAFE_SOCKET(NULL, AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

	memset(&ic6, 0, sizeof(ic6));
	ic6.icmp6_type = type;
	ic6.icmp6_data32[0] = htonl(getpid());

	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = in6addr_loopback;
	if (sendto(s, &ic6, sizeof(ic6), 0, (struct sockaddr *)&sin6,
		   sizeof(sin6)) == -1) {
		tst_resm(TBROK | TERRNO, "%s: sendto failed", tname);
		return 1;
	}
	return 0;
}

static int ic6_recv1(char *tname, int sall, int sf)
{
	fd_set readfds, readfds_saved;
	struct timeval tv;
	int maxfd, nfds;
	int gotall, gotone;
	int cc;
	static unsigned char rbuf[2048];

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
		struct icmp6_hdr *pic6 = (struct icmp6_hdr *)rbuf;

		nfds = select(maxfd + 1, &readfds, 0, 0, &tv);
		if (nfds == 0)
			break;	/* timed out */
		if (nfds < 0) {
			if (errno == EINTR)
				continue;
			tst_resm(TBROK | TERRNO, "%s: select failed", tname);
		}
		if (FD_ISSET(sall, &readfds)) {
			cc = recv(sall, rbuf, sizeof(rbuf), 0);
			if (cc < 0) {
				tst_resm(TBROK | TERRNO,
					 "%s: recv(sall, ..) failed", tname);
				return -1;
			}
			/* if packet check succeeds... */
			if (htonl(pic6->icmp6_data32[0]) == (uint32_t)getpid())
				gotall = 1;
		}
		if (FD_ISSET(sf, &readfds)) {
			cc = recv(sf, rbuf, sizeof(rbuf), 0);
			if (cc < 0) {
				tst_resm(TBROK | TERRNO,
					 "%s: recv(sf, ..) failed", tname);
				return -1;
			}
			/* if packet check succeeds... */
			if (htonl(pic6->icmp6_data32[0]) == (uint32_t)getpid())
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
static void icmp6_ft(void)
{
	struct icmp6_filter i6f;
	int sall, sf;
	unsigned int i;

	sall = SAFE_SOCKET(NULL, PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

	ICMP6_FILTER_SETPASSALL(&i6f);
	if (setsockopt(sall, IPPROTO_ICMPV6, ICMP6_FILTER, &i6f,
		       sizeof(i6f)) < 0) {
		tst_resm(TBROK | TERRNO,
			 "setsockopt pass all ICMP6_FILTER failed");
	}

	sf = SAFE_SOCKET(NULL, PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

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
				tst_resm(TFAIL | TERRNO,
					 "setsockopt ICMP6_FILTER");
				continue;
			}
			if (ic6_send1(ftab[i].ft_tname, ftab[i].ft_sndtype))
				continue;
			rv = ic6_recv1(ftab[i].ft_tname, sall, sf);
		}

		if (rv < 0)
			continue;
		if (rv != ftab[i].ft_expected)
			tst_resm(TFAIL, "%s: rv %d != expected %d",
				 ftab[i].ft_tname, rv, ftab[i].ft_expected);
		else
			tst_resm(TPASS, "%s", ftab[i].ft_tname);
	}
}
