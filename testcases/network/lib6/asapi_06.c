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
 * Test Name: asapi_06
 *
 * Test Description:
 *  Tests for RFC 3542 section 4 socket options and ancillary data
 *
 * Usage:  <for command-line>
 *  asapi_06
 *
 * HISTORY
 *	05/2005 written by David L Stevens
 *
 * RESTRICTIONS:
 *  None.
 *
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
#include "usctest.h"

char *TCID="asapi_06";		/* Test program identifier.    */

int TST_TOTAL = 1;

pid_t pid;

struct {
	char	*prt_name;
	int	prt_value;
} ptab[] = {
};

#define PTCOUNT	(sizeof(ptab)/sizeof(ptab[0]))

#define READ_TIMEOUT	5	/* secs */

void do_tests(void);
void setup(void), cleanup(void);

int
main(int argc, char *argv[])
{
	char *msg;		/* message returned from parse_opts */
	int lc;

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, 0, 0);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	pid = getpid();

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc)
		do_tests();

	cleanup();

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
	struct in6_pktinfo	sou_pktinfo;
	int			sou_hoplimit;
	struct sockaddr_in6	sou_nexthop;
	struct ip6_rthdr	sou_rthdr;
	struct ip6_hbh		sou_hopopts;
	struct ip6_dest		sou_dstopts;
	struct ip6_dest		sou_rthdrdstopts;
	int			sou_tclass;
	int			sou_bool;
};

/* in6_addr initializer for loopback interface */
#define IN6_LOOP	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }
#define IN6_ANY		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

/* so_clrval and so_setval members are initilized in the body */
struct soent {
	char		*so_tname;
	int		so_opt;
	int		so_dorecv;	/* do receive test? */
	int		so_cmtype;
	int		so_clear;	/* get fresh socket? */
	union soval	so_clrval;
	union soval	so_setval;
	socklen_t	so_valsize;
} sotab[] = {
/* RFC 3542, Section 4 */
	{ "IPV6_RECVPKTINFO", IPV6_RECVPKTINFO, 1, IPV6_PKTINFO, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_RECVHOPLIMIT", IPV6_RECVHOPLIMIT, 1, IPV6_HOPLIMIT, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_RECVRTHDR", IPV6_RECVRTHDR, 0, IPV6_RTHDR, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_RECVHOPOPTS", IPV6_RECVHOPOPTS, 0, IPV6_HOPOPTS, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_RECVDSTOPTS", IPV6_RECVDSTOPTS, 0, IPV6_DSTOPTS, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_RECVTCLASS", IPV6_RECVTCLASS, 1, IPV6_TCLASS, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
/* make sure TCLASS stays when setting another opt */
	{ "IPV6_RECVTCLASS (2)", IPV6_RECVHOPLIMIT, 1, IPV6_TCLASS, 0,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
/* OLD values */
	{ "IPV6_2292PKTINFO", IPV6_2292PKTINFO, 1, IPV6_2292PKTINFO, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_2292HOPLIMIT", IPV6_2292HOPLIMIT, 1, IPV6_2292HOPLIMIT, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_2292RTHDR", IPV6_2292RTHDR, 0, IPV6_2292RTHDR, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_2292HOPOPTS", IPV6_2292HOPOPTS, 0, IPV6_2292HOPOPTS, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
	{ "IPV6_2292DSTOPTS", IPV6_2292DSTOPTS, 0, IPV6_2292DSTOPTS, 1,
		{{{{{0}}}}}, {{{{{0}}}}}, sizeof(int) },
};

#define SOCOUNT	(sizeof(sotab)/sizeof(sotab[0]))

struct soprot {
	int		sop_pid;	/* sender PID */
	int		sop_seq;	/* sequence # */
	int		sop_dlen;	/* tp_dat length */
	unsigned char	sop_dat[0];	/* user data */
};

unsigned char tpbuf[sizeof(struct soprot) + 2048];
unsigned char rpbuf[sizeof(struct soprot) + 2048];

unsigned char control[2048];
int clen;

int seq;

int
setupso(void)
{
/* add routing headers, other ancillary data here */
	return 0;
}

struct cme {
	int	cm_len;
	int	cm_level;
	int	cm_type;
	union {
	        uint32_t cmu_tclass;
		uint32_t cmu_hops;
	} cmu;
} cmtab[] = {
	{ sizeof(uint32_t), SOL_IPV6, IPV6_TCLASS, {0x12} },
	{ sizeof(uint32_t), SOL_IPV6, IPV6_HOPLIMIT, {0x21} },
};

#define CMCOUNT	(sizeof(cmtab)/sizeof(cmtab[0]))

ssize_t
sendall(int st)
{
	struct sockaddr_in6 sin6;
	struct msghdr msg;
	struct iovec iov;
	struct soprot	*psop;
	unsigned char *pd;
	int i, ctotal;

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
	for (i=0; i<CMCOUNT; ++i) {
		struct cmsghdr *pcmsg = (struct cmsghdr *) pd;

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

void
so_test(struct soent *psoe)
{
	struct sockaddr_in6 sin6;
	union soval	sobuf;
        socklen_t valsize;
	static int sr = -1;
	int st;

	if (psoe->so_opt == -1) {
		tst_resm(TBROK, "%s not present at compile time",
			psoe->so_tname);
		return;
	}
	if (psoe->so_clear || sr < 0) {
		if (sr <  0)
			close(sr);
		sr = socket(PF_INET6, SOCK_RAW, NH_TEST);
		if (sr < 0) {
			tst_resm(TBROK, "%s socket: %s", psoe->so_tname,
				strerror(errno));
			return;
		}
	}
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = in6addr_loopback;
	if (bind(sr, (struct sockaddr *)&sin6, sizeof(sin6)) < 0) {
		tst_resm(TBROK, "%s: bind: %s", psoe->so_tname,
			strerror(errno));
	}
	if (setsockopt(sr, SOL_IPV6, psoe->so_opt, &psoe->so_clrval,
	    psoe->so_valsize) < 0) {
		tst_resm(TBROK, "%s: setsockopt: %s", psoe->so_tname,
			strerror(errno));
		return;
	}
	TEST(setsockopt(sr, SOL_IPV6, psoe->so_opt, &psoe->so_setval,
		psoe->so_valsize));
	if (TEST_RETURN != 0) {
		tst_resm(TFAIL, "%s set-get: setsockopt: %s", psoe->so_tname,
			strerror(errno));
		return;
	}
	valsize = psoe->so_valsize;
	TEST(getsockopt(sr, SOL_IPV6, psoe->so_opt, &sobuf, &valsize));
	if (TEST_RETURN != 0) {
		tst_resm(TBROK, "%s set-get: getsockopt: %s", psoe->so_tname,
				strerror(errno));
		return;
	} else if (memcmp(&psoe->so_setval, &sobuf, psoe->so_valsize))
		tst_resm(TFAIL, "%s set-get optval != setval", psoe->so_tname);
	else
		tst_resm(TPASS, "%s set-get", psoe->so_tname);

	st = socket(PF_INET6, SOCK_RAW, NH_TEST);
	if (st < 0) {
		tst_resm(TBROK, "%s transmit socket: %s", psoe->so_tname,
			strerror(errno));
		return;
	}
	if (sendall(st) < 0) {
		tst_resm(TBROK, "%s transmit sendto: %s", psoe->so_tname,
			strerror(errno));
		close(st);
		return;
	}
	close(st);

	/* receiver processing */
	{ fd_set rfds, rfds_saved;
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
			nfds = select(sr+1, &rfds, 0, 0, &tv);
			if (nfds < 0) {
				if (errno == EINTR)
					continue;
				tst_resm(TBROK, "%s select: %s", psoe->so_tname,
					strerror(errno));
				return;
			}
			if (nfds == 0) {
				tst_resm(TBROK, "%s recvmsg timed out",
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
			if (cc  < 0) {
				tst_resm(TBROK, "%s recvmsg: %s",
					psoe->so_tname, strerror(errno));
				return;
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
			if (gotone)
				break;
			else if (psoe->so_clear) {
				tst_resm(TFAIL, "%s receive: extraneous data "
					"in control: level %d type %d len %d",
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

#define IPV6_ADDR_NODE		1
#define IPV6_ADDR_LINK		2
#define IPV6_ADDR_GLOBAL	3

#ifdef HAVE_IFADDRS_H
static int ipv6_addr_scope(struct in6_addr *pin6)
{
	if ((ntohl(pin6->s6_addr32[0]) & 0xFFC00000) == 0xFE800000)
		return IPV6_ADDR_LINK;
	if (memcmp(pin6, &in6addr_loopback, sizeof(*pin6)) == 0)
		return IPV6_ADDR_NODE;
	return IPV6_ADDR_GLOBAL;
}
#endif	/* HAVE_IFADDRS_H */

int getsock(char *tname, struct sockaddr_in6 *psin6_arg, int scope)
{
#ifdef HAVE_IFADDRS_H
	static struct ifaddrs *pifa_head;
	struct ifaddrs *pifa;
	struct sockaddr_in6 *psin6;
	char strbuf[128];
	int ifindex = 0;
	int s;

	if (!pifa_head && getifaddrs(&pifa_head)) {
		tst_resm(TBROK, "%s: getifaddrs failed", tname);
		return -1;
	}
	if (psin6_arg)
		ifindex = psin6_arg->sin6_scope_id;

	/* first, find a global address */
	for (pifa=pifa_head; pifa; pifa=pifa->ifa_next) {
		int this_scope;

		if (!(pifa->ifa_flags & IFF_UP))
			continue;
		if (pifa->ifa_addr->sa_family != AF_INET6)
			continue;
		psin6 = (struct sockaddr_in6 *)pifa->ifa_addr;
		this_scope = ipv6_addr_scope(&psin6->sin6_addr);
		if (this_scope &&
		    ((this_scope < 0 && -this_scope == scope) ||
		     (this_scope > 0 && this_scope != scope)))
			continue;
		psin6->sin6_scope_id = if_nametoindex(pifa->ifa_name);
		if ((ifindex < 0 && -ifindex == psin6->sin6_scope_id) ||
		    (ifindex > 0 && ifindex != psin6->sin6_scope_id))
			continue;
		s = socket(PF_INET6, SOCK_DGRAM, 0);
		if (s < 0) {
			tst_resm(TBROK, "%s: socket %s", tname,strerror(errno));
			return -1;
		}
		if (bind(s, pifa->ifa_addr, sizeof(struct sockaddr_in6)) < 0) {
			tst_resm(TBROK, "%s: bind \"%s\": %s", tname,
				inet_ntop(AF_INET6, &psin6->sin6_addr, strbuf,
					sizeof(strbuf)), strerror(errno));
			return -1;
		}
		if (psin6_arg) {
			*psin6_arg = *psin6;
			psin6_arg->sin6_scope_id=if_nametoindex(pifa->ifa_name);
		}
		return s;
	}
	{
		char *scopestr, *intfstr;

		switch (scope) {
		case IPV6_ADDR_NODE:	scopestr = " node-local"; break;
		case IPV6_ADDR_LINK:	scopestr = " link-local"; break;
		case IPV6_ADDR_GLOBAL:	scopestr = " global"; break;
		default:
			scopestr = ""; break;
		}
		if (ifindex < 0) {
			intfstr = " not on ifindex";
			ifindex = -ifindex;
		} else if (ifindex)
			intfstr = " on ifindex";
		else
			intfstr = 0;

		if (intfstr)
			tst_resm(TBROK, "%s: getsock : no%s addresses%s %d",
				tname, scopestr, intfstr, ifindex);
		else
			tst_resm(TBROK, "%s: getsock : no%s addresses",
				tname,  scopestr);
	}
	return -1;
#else /* HAVE_IFADDRS_H */
	return -1;
#endif
}

#ifdef notyet
/*
 * RFC 3542 IPV6_PKTINFO not in mainline yet (as of 2.6.15). The get/set
 * tests are below, and comments for some further tests to be added later
 */
void test_pktinfo(void)
{
	int s_snd, s_rcv[3] = {-1, -1, -1 };
	struct sockaddr_in6	sa_rcv[3];
	int s, i;
	struct ifaddrs *pifa_head, *pifa;
	struct sockaddr_in6 *psin6;
	char strbuf[128];
	char *tname = "IPV6_PKTINFO";
	struct in6_pktinfo	pi, pi_tmp;
	int sinlen;
	int optlen;

	s_snd = getsock(tname, 0, IPV6_ADDR_GLOBAL);
	if (s_snd < 0) {
		tst_resm(TBROK, "%s: can't create send socket", tname);
		return;
	}
	/* global-scope address, interface X */
	sa_rcv[0].sin6_scope_id = 0;
	s_rcv[0] = getsock(tname, &sa_rcv[0], IPV6_ADDR_GLOBAL);
	if (s_rcv[0] == -1) {
		tst_resm(TBROK, "%s: only link-scope addresses", tname);
		return;
	}
	/* link-local-scope address, interface X */
	sa_rcv[1].sin6_scope_id = sa_rcv[0].sin6_scope_id;
	s_rcv[1] = getsock(tname, &sa_rcv[1], IPV6_ADDR_LINK);
	if (s_rcv[1] < 0) {
		tst_resm(TBROK, "%s: no link-local address on ifindex %d",
			tname, sa_rcv[0].sin6_scope_id);
		return;
	}
	/* link-local-scope address, interface Y */
	sa_rcv[2].sin6_scope_id = -sa_rcv[0].sin6_scope_id;
	s_rcv[2] = getsock(tname, &sa_rcv[2], IPV6_ADDR_LINK);
	if (s_rcv[2] < 0) {
		tst_resm(TBROK, "%s: only one interface?", tname);
		return;
	}
	/* send to rcv1 to verify communication */
	/* force to rcv2 w/ PKTINFO */
/* TESTS: */
/* sticky set-get */
	tname = "IPV6_PKTINFO set";
	pi.ipi6_addr = sa_rcv[1].sin6_addr;
	pi.ipi6_ifindex = sa_rcv[1].sin6_scope_id;
	TEST(setsockopt(s_snd, SOL_IPV6, IPV6_PKTINFO, &pi, sizeof(pi)));
	if (TEST_RETURN != 0)
		tst_resm(TFAIL, "%s: %s", tname, strerror(errno));
	else
		tst_resm(TPASS, "%s", tname);

	tname = "IPV6_PKTINFO get";
	optlen = sizeof(pi_tmp);
	TEST(getsockopt(s_snd, SOL_IPV6, IPV6_PKTINFO, &pi_tmp, &optlen));
	if (TEST_RETURN != 0)
		tst_resm(TFAIL, "%s: %s", tname, strerror(errno));
	else if (memcmp(&pi, &pi_tmp, sizeof(pi)) != 0) {
		char strbuf2[64];
		tst_resm(TFAIL, "%s: {\"%s\",%d} != {\"%s\",%d}", tname,
			inet_ntop(AF_INET6, &pi_tmp.ipi6_addr, strbuf,
				sizeof(strbuf)), pi_tmp.ipi6_ifindex,
			inet_ntop(AF_INET6, &pi.ipi6_addr, strbuf2,
				sizeof(strbuf2)), pi.ipi6_ifindex);
	} else
		tst_resm(TPASS, "%s", tname);
/* ancillary data override */
/* link-local, wrong interface */
	tname = "IPV6_PKTINFO invalid {lladdr, intf}";
	pi.ipi6_addr = sa_rcv[1].sin6_addr;
	pi.ipi6_ifindex = sa_rcv[2].sin6_scope_id;
	TEST(setsockopt(s_snd, SOL_IPV6, IPV6_PKTINFO, &pi, sizeof(pi)));
	if (TEST_RETURN == 0)
		tst_resm(TFAIL, "%s returns success, should be -1, EINVAL",
			tname);
	else if (TEST_ERRNO != EINVAL)
		tst_resm(TFAIL, "%s errno %d != %d", tname, TEST_ERRNO, EINVAL);
	else
		tst_resm(TPASS, "%s", tname);
/* nonexistent interface */
/* non-local address */
/* clear address */
/* clear interface */
/* sendmsg() sin6_scope differs with ancillary data interface */
}
#endif /* notyet */

void
do_tests(void)
{
	int	i;

	for (i=0; i<SOCOUNT; ++i) {
		sotab[i].so_clrval.sou_bool = 0;
		sotab[i].so_setval.sou_bool = 1;
		so_test(&sotab[i]);
	}
#ifdef notyet
	test_pktinfo();
#endif /* notyet - see test_pktinfo() comment above */
}

void
setup(void)
{
	TEST_PAUSE;	/* if -P option specified */
}

void
cleanup(void)
{
	TEST_CLEANUP;

}