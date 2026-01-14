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

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif
#include <arpa/inet.h>

#include "test.h"
#include "tso_safe_macros.h"

char *TCID = "asapi_03";

int TST_TOTAL = 1;

#define READ_TIMEOUT	5	/* secs */

static void do_tests(void);
static void setup(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc)
		do_tests();

	tst_exit();
}

#define NH_TEST	0x9f

#ifndef IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO	-1
#endif
#ifndef IPV6_RECVHOPLIMIT
#define IPV6_RECVHOPLIMIT	-1
#endif
#ifndef IPV6_RECVRTHDR
#define IPV6_RECVRTHDR		-1
#endif
#ifndef IPV6_RECVHOPOPTS
#define IPV6_RECVHOPOPTS	-1
#endif
#ifndef IPV6_RECVDSTOPTS
#define IPV6_RECVDSTOPTS	-1
#endif
#ifndef IPV6_RECVTCLASS
#define IPV6_RECVTCLASS		-1
#endif
#ifndef IPV6_TCLASS
#define IPV6_TCLASS		-1
#endif
#ifndef IPV6_2292PKTINFO
#define	IPV6_2292PKTINFO	-1
#endif
#ifndef IPV6_2292HOPLIMIT
#define	IPV6_2292HOPLIMIT	-1
#endif
#ifndef IPV6_2292RTHDR
#define	IPV6_2292RTHDR		-1
#endif
#ifndef IPV6_2292HOPOPTS
#define	IPV6_2292HOPOPTS	-1
#endif
#ifndef IPV6_2292DSTOPTS
#define	IPV6_2292DSTOPTS	-1
#endif

union soval {
	struct in6_pktinfo sou_pktinfo;
	int sou_hoplimit;
	struct sockaddr_in6 sou_nexthop;
	struct ip6_rthdr sou_rthdr;
	struct ip6_hbh sou_hopopts;
	struct ip6_dest sou_dstopts;
	struct ip6_dest sou_rthdrdstopts;
	int sou_tclass;
	int sou_bool;
};

/* in6_addr initializer for loopback interface */
#define IN6_LOOP	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }
#define IN6_ANY		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

/* so_clrval and so_setval members are initilized in the body */
static struct soent {
	char *so_tname;
	int so_opt;
	int so_dorecv;		/* do receive test? */
	int so_cmtype;
	int so_clear;		/* get fresh socket? */
	union soval so_clrval;
	union soval so_setval;
	socklen_t so_valsize;
} sotab[] = {
	/* RFC 3542, Section 4 */
	{"IPV6_RECVPKTINFO", IPV6_RECVPKTINFO, 1, IPV6_PKTINFO, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_RECVHOPLIMIT", IPV6_RECVHOPLIMIT, 1, IPV6_HOPLIMIT, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_RECVRTHDR", IPV6_RECVRTHDR, 0, IPV6_RTHDR, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_RECVHOPOPTS", IPV6_RECVHOPOPTS, 0, IPV6_HOPOPTS, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_RECVDSTOPTS", IPV6_RECVDSTOPTS, 0, IPV6_DSTOPTS, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_RECVTCLASS", IPV6_RECVTCLASS, 1, IPV6_TCLASS, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	/* make sure TCLASS stays when setting another opt */
	{"IPV6_RECVTCLASS (2)", IPV6_RECVHOPLIMIT, 1, IPV6_TCLASS, 0,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	/* OLD values */
	{"IPV6_2292PKTINFO", IPV6_2292PKTINFO, 1, IPV6_2292PKTINFO, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_2292HOPLIMIT", IPV6_2292HOPLIMIT, 1, IPV6_2292HOPLIMIT, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_2292RTHDR", IPV6_2292RTHDR, 0, IPV6_2292RTHDR, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_2292HOPOPTS", IPV6_2292HOPOPTS, 0, IPV6_2292HOPOPTS, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
	{"IPV6_2292DSTOPTS", IPV6_2292DSTOPTS, 0, IPV6_2292DSTOPTS, 1,
	 {{{{{0} } }, 0} }, {{{{{0} } }, 0} }, sizeof(int)},
};

#define SOCOUNT	ARRAY_SIZE(sotab)

struct soprot {
	int sop_pid;			/* sender PID */
	int sop_seq;			/* sequence # */
	int sop_dlen;			/* tp_dat length */
	unsigned char sop_dat[0];	/* user data */
};

static unsigned char tpbuf[sizeof(struct soprot) + 2048];
static unsigned char rpbuf[sizeof(struct soprot) + 2048];

static unsigned char control[2048];

static int seq;

static struct cme {
	int cm_len;
	int cm_level;
	int cm_type;
	union {
		uint32_t cmu_tclass;
		uint32_t cmu_hops;
	} cmu;
} cmtab[] = {
	{sizeof(uint32_t), SOL_IPV6, IPV6_TCLASS, {0x12} },
	{sizeof(uint32_t), SOL_IPV6, IPV6_HOPLIMIT, {0x21} },
};

#define CMCOUNT	ARRAY_SIZE(cmtab)

static ssize_t sendall(int st)
{
	struct sockaddr_in6 sin6;
	struct msghdr msg;
	struct iovec iov;
	struct soprot *psop;
	unsigned char *pd;
	unsigned int i;
	int ctotal;

	psop = (struct soprot *)tpbuf;
	psop->sop_pid = htonl(getpid());
	psop->sop_seq = ++seq;
	psop->sop_dlen = 0;

	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = in6addr_loopback;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &sin6;
	msg.msg_namelen = sizeof(sin6);
	iov.iov_base = tpbuf;
	iov.iov_len = sizeof(struct soprot) + ntohl(psop->sop_dlen);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	pd = control;
	ctotal = 0;
	for (i = 0; i < CMCOUNT; ++i) {
		struct cmsghdr *pcmsg = (struct cmsghdr *)pd;

		pcmsg->cmsg_len = CMSG_LEN(cmtab[i].cm_len);
		pcmsg->cmsg_level = cmtab[i].cm_level;
		pcmsg->cmsg_type = cmtab[i].cm_type;
		memcpy(CMSG_DATA(pcmsg), &cmtab[i].cmu, cmtab[i].cm_len);
		pd += CMSG_SPACE(cmtab[i].cm_len);
		ctotal += CMSG_SPACE(cmtab[i].cm_len);
	}
	msg.msg_control = ctotal ? control : 0;
	msg.msg_controllen = ctotal;

	return sendmsg(st, &msg, 0);
}

static void so_test(struct soent *psoe)
{
	struct sockaddr_in6 sin6;
	union soval sobuf;
	socklen_t valsize;
	static int sr = -1;
	int st;

	if (psoe->so_opt == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "%s not present at compile time",
			 psoe->so_tname);
	}
	if (psoe->so_clear || sr < 0) {
		if (sr < 0)
			close(sr);
		sr = SAFE_SOCKET(NULL, PF_INET6, SOCK_RAW, NH_TEST);
	}
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = in6addr_loopback;

	SAFE_BIND(NULL, sr, (struct sockaddr *)&sin6, sizeof(sin6));

	if (setsockopt(sr, SOL_IPV6, psoe->so_opt, &psoe->so_clrval,
		       psoe->so_valsize) < 0) {
		tst_brkm(TBROK | TERRNO, NULL, "%s: setsockopt",
			 psoe->so_tname);
	}

	TEST(setsockopt(sr, SOL_IPV6, psoe->so_opt, &psoe->so_setval,
			psoe->so_valsize));
	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "%s set-get: setsockopt",
			 psoe->so_tname);
		return;
	}

	valsize = psoe->so_valsize;
	TEST(getsockopt(sr, SOL_IPV6, psoe->so_opt, &sobuf, &valsize));
	if (TEST_RETURN != 0) {
		tst_brkm(TBROK | TTERRNO, NULL, "%s set-get: getsockopt",
			 psoe->so_tname);
	} else if (memcmp(&psoe->so_setval, &sobuf, psoe->so_valsize)) {
		tst_resm(TFAIL, "%s set-get optval != setval", psoe->so_tname);
	} else {
		tst_resm(TPASS, "%s set-get", psoe->so_tname);
	}

	st = SAFE_SOCKET(NULL, PF_INET6, SOCK_RAW, NH_TEST);

	if (sendall(st) < 0)
		tst_brkm(TBROK | TERRNO, NULL, "%s transmit sendto",
			 psoe->so_tname);

	close(st);

	/* receiver processing */
	{
		fd_set rfds, rfds_saved;
		int nfds, cc;
		int gotone;
		struct timeval tv;
		struct msghdr msg;
		unsigned char cmsg[2048];
		struct cmsghdr *pcmsg;
		struct iovec iov;

		FD_ZERO(&rfds_saved);
		FD_SET(sr, &rfds_saved);

		tv.tv_sec = 0;
		tv.tv_usec = 250000;

		while (1) {
			memcpy(&rfds, &rfds_saved, sizeof(rfds));
			nfds = select(sr + 1, &rfds, 0, 0, &tv);
			if (nfds < 0) {
				if (errno == EINTR)
					continue;
				tst_brkm(TBROK | TERRNO, NULL, "%s select",
					 psoe->so_tname);
			}
			if (nfds == 0) {
				tst_brkm(TBROK, NULL, "%s recvmsg timed out",
					 psoe->so_tname);
				return;
			}
			/* else, nfds == 1 */
			if (!FD_ISSET(sr, &rfds))
				continue;

			memset(&msg, 0, sizeof(msg));
			iov.iov_base = rpbuf;
			iov.iov_len = sizeof(rpbuf);
			msg.msg_iov = &iov;
			msg.msg_iovlen = 1;
			msg.msg_control = cmsg;
			msg.msg_controllen = sizeof(cmsg);

			cc = recvmsg(sr, &msg, 0);
			if (cc < 0) {
				tst_brkm(TBROK | TERRNO, NULL, "%s recvmsg",
					 psoe->so_tname);
			}
			/* check pid & seq here */
			break;
		}
		gotone = 0;
		for (pcmsg = CMSG_FIRSTHDR(&msg); pcmsg != NULL;
		     pcmsg = CMSG_NXTHDR(&msg, pcmsg)) {
			if (!psoe->so_dorecv)
				break;
			gotone = pcmsg->cmsg_level == SOL_IPV6 &&
			    pcmsg->cmsg_type == psoe->so_cmtype;
			if (gotone) {
				break;
			} else if (psoe->so_clear) {
				tst_resm(TFAIL, "%s receive: extraneous data "
					 "in control: level %d type %d len %zu",
					 psoe->so_tname, pcmsg->cmsg_level,
					 pcmsg->cmsg_type, pcmsg->cmsg_len);
				return;
			}
		}
		/* check contents here */
		if (psoe->so_dorecv)
			tst_resm(gotone ? TPASS : TFAIL, "%s receive",
				 psoe->so_tname);
	}
}

static void do_tests(void)
{
	unsigned int i;

	for (i = 0; i < SOCOUNT; ++i) {
		sotab[i].so_clrval.sou_bool = 0;
		sotab[i].so_setval.sou_bool = 1;
		so_test(&sotab[i]);
	}
}

static void setup(void)
{
	TEST_PAUSE;
}
