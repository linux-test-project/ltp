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
 * Test Name: asapi_04
 *
 * Test Description:
 *  Verify that in6 and sockaddr fields are present. Most of these are
 *  "PASS" if they just compile.
 *
 * Usage:  <for command-line>
 *  asapi_04
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
#include <netdb.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

char *TCID="asapi_04";		/* Test program identifier.    */

pid_t pid;

struct {
	char	*prt_name;
	int	prt_value;
} ptab[] = {
	{ "hopopt", 0 },
	{ "ipv6", 41 },
	{ "ipv6-route", 43 },
	{ "ipv6-frag", 44 },
	{ "esp", 50 },
	{ "ah", 51 },
	{ "ipv6-icmp", 58 },
	{ "ipv6-nonxt", 59 },
	{ "ipv6-opts", 60},
};

#define PTCOUNT	(sizeof(ptab)/sizeof(ptab[0]))

#define READ_TIMEOUT	5	/* secs */

void do_tests(void);
void setup(void), cleanup(void);
int csum_test(char *rhost);

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

void
do_tests(void)
{
	int i;

/* RFC 3542, Section 2.3 */
#ifndef IN6_ARE_ADDR_EQUAL
	tst_resm(TBROK, "IN6_ARE_ADDR_EQUAL not present");
#else /* IN6_ARE_ADDR_EQUAL */
	/*
	 * set each bit in an address and check for unequal; then set
	 * in the second address and check for equal. Covers all bits, all
	 * combinations.
	 */
	{ struct in6_addr a1, a2;
	  int word, bit;
	  int rv = 1;

		memset(&a1, 0, sizeof(a1));
		memset(&a2, 0, sizeof(a2));

		rv = IN6_ARE_ADDR_EQUAL(&a1, &a2);

		for (word=0; word<4; ++word)
			for (bit=0; bit<32; ++bit) {
				uint32_t newbit = 1<<bit;

				a1.s6_addr32[word] |= newbit;	/* unequal */
				rv &= !IN6_ARE_ADDR_EQUAL(&a1, &a2);
				a2.s6_addr32[word] |= newbit;	/* equal */
				rv &= IN6_ARE_ADDR_EQUAL(&a1, &a2);
			}
		tst_resm(rv ? TPASS : TFAIL, "IN6_ARE_ADDR_EQUAL");
	}
#endif /* IN6_ARE_ADDR_EQUAL */

/* RFC 3542, Section 2.4 */
	for (i=0; i < PTCOUNT; ++i) {
		struct protoent *pe;
		int	pass;

		pe = getprotobyname(ptab[i].prt_name);
		pass = pe && pe->p_proto == ptab[i].prt_value;
		tst_resm(pass ? TPASS : TFAIL, "\"%s\" protocols entry",
			ptab[i].prt_name);
	}
/* RFC 3542, Section 3.1 */
	csum_test("::1");
}

/*
 * this next-header value shouldn't be a real protocol!!
 * 0x9f = 01 0 11111
 *         | |     |
 *         | |     |--- rest- ~0
 *         | |--------- chg - "no change enroute"
 *         |----------- act - "discard unknown"
 */
#define	NH_TEST	0x9f

struct tprot {
	int		tp_pid;		/* sender PID */
	int		tp_seq;		/* sequence # */
	int		tp_offset;	/* offset of cksum */
	int		tp_dlen;	/* tp_dat length */
	unsigned char	tp_dat[0];	/* user data */
};

unsigned char tpbuf[sizeof(struct tprot) + 2048];
unsigned char rpbuf[sizeof(struct tprot) + 2048];

struct csent {
	int	 cs_offset;
	int	 cs_dlen;
	int	 cs_setresult;	/* setsockopt expected result */
	int	 cs_seterrno;	/* setsockopt expected errno */
	int	 cs_sndresult;	/* send expected result */
	int	 cs_snderrno;	/* send expected errno */
} cstab[] = {
	{ 0, 5, 0, 0, 0, 0 },
	{ 6, 30, 0, 0, 0, 0 },
	{ 3, 20, -1, EINVAL, -1, -1 },	/* non-aligned offset */
	{ 4, 5, 0, 0, -1, EINVAL },		/* not enough space */
	{ 50, 5, 0, 0, -1, EINVAL },	/* outside of packet */
	{ 22, 30, 0, 0, 0, 0 },
	{ 2000, 2004, 0, 0, 0, 0 },	/* in a fragment (over Ethernet) */
};

#define CSCOUNT	(sizeof(cstab)/sizeof(cstab[0]))

static int recvtprot(int sd, unsigned char *packet, int psize)
{
	struct tprot *tpt;
	int cc, total, expected;
	int gothead;

	tpt = (struct tprot *)packet;
	total = cc = recv(sd, packet, sizeof(struct tprot), 0);
	expected = sizeof(struct tprot);	/* until we get tp_dlen */
	gothead = total >= sizeof(struct tprot);
	if (gothead)
		expected += ntohl(tpt->tp_dlen);
	if (cc <= 0)
		return cc;
	while (cc > 0 && total < expected) {
		cc = recv(sd, &packet[total], expected-total, 0);
		if (cc >= 0) {
			total += cc;
			if (!gothead && total >= sizeof(struct tprot)) {
				gothead = 1;
				expected += ntohl(tpt->tp_dlen);
			}
		} else
			break;
	}
	if (cc < 0)
		return cc;
	return total;
}

unsigned short csum(unsigned short partial, unsigned char *packet, int len)
{
	unsigned long sum = partial;
	unsigned short *ps;
	int i;

	ps = (unsigned short *)packet;
	for (i=0; i<len/2; ++i)
		sum += *ps++;
	if (len & 1)
		sum += htons(packet[len-1]<<8);
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ~sum;
}

struct ph {
	struct in6_addr ph_sa;
	struct in6_addr ph_da;
	uint32_t ph_len;
	uint8_t ph_mbz[3];
	uint8_t ph_nh;
} ph;

static int client(int prot, int sfd)
{
	struct tprot *pttp = (struct tprot *)tpbuf;
	struct tprot *prtp = (struct tprot *)rpbuf;
	struct sockaddr_in6 rsin6;
	static int seq;
	int i, sd, cc, cs;

	memset(&rsin6, 0, sizeof(rsin6));
	rsin6.sin6_family = AF_INET6;
	rsin6.sin6_addr = in6addr_loopback;

	memset(&ph, 0, sizeof(ph));
	ph.ph_sa = rsin6.sin6_addr;
	ph.ph_da = rsin6.sin6_addr;
	ph.ph_nh = NH_TEST;

	sd = socket(PF_INET6, SOCK_RAW, NH_TEST);
	if (sd < 0) {
		tst_resm(TBROK, "can't create raw socket: %s", strerror(errno));
		return -1;
	}
	for (i=0; i<CSCOUNT; ++i) {
		int offset, len, xlen;
		int rv;
		unsigned char *p, *pend;

		offset = sizeof(struct tprot) + cstab[i].cs_offset;
		len = sizeof(struct tprot) + cstab[i].cs_dlen;

		memset(pttp, 0, sizeof(*pttp));
		memset(pttp->tp_dat, 0xA5, cstab[i].cs_dlen);

		pttp->tp_pid = htonl(pid);
		pttp->tp_offset = ntohl(offset);
		pttp->tp_dlen = ntohl(cstab[i].cs_dlen);
		pttp->tp_seq = ntohl(++seq);

		TEST(setsockopt(sd, IPPROTO_IPV6, IPV6_CHECKSUM, &offset,
				sizeof(offset)));
		if (TEST_RETURN != cstab[i].cs_setresult) {
			tst_resm(TFAIL|TTERRNO, "IPV6_CHECKSUM offset %d len %d "
				"- result %ld != %d", offset, len,
				TEST_RETURN, cstab[i].cs_setresult);
			continue;
		}
		if (TEST_RETURN < 0) {
			tst_resm(TPASS, "IPV6_CHECKSUM offset %d len %d",
				offset, len);
			continue;
		}
		if (TEST_RETURN && TEST_ERRNO != cstab[i].cs_seterrno) {
			tst_resm(TFAIL, "IPV6_CHECKSUM offset %d len %d "
				"- errno %d != %d", offset, len,
				TEST_ERRNO, cstab[i].cs_seterrno);
			continue;
		}
		/* send packet */
		TEST(sendto(sd, pttp, len, 0, (struct sockaddr *)&rsin6,
			sizeof(rsin6)));
		xlen = (cstab[i].cs_sndresult < 0) ? -1 : len;
		if (TEST_RETURN != xlen) {
			tst_resm(TFAIL|TTERRNO, "IPV6_CHECKSUM offset %d len %d "
				"- sndresult %ld != %d",
				offset, len, TEST_RETURN, xlen);
			continue;
		}
		if (TEST_RETURN < 0 && TEST_ERRNO != cstab[i].cs_snderrno) {
			tst_resm(TFAIL, "IPV6_CHECKSUM offset %d len %d "
				"- snderrno %d != %d", offset, len,
				TEST_ERRNO, cstab[i].cs_snderrno);
			continue;
		}
		if (TEST_RETURN < 0) {
			tst_resm(TPASS, "IPV6_CHECKSUM offset %d len %d",
				offset, len);
			continue;
		}
		while ((cc = recvtprot(sfd, rpbuf, sizeof(rpbuf)))) {
			if (htonl(prtp->tp_pid) == pid &&
			    htonl(prtp->tp_seq) == seq)
				break;
		}
		rv = 1;
		pend = rpbuf + sizeof(struct tprot) + ntohl(prtp->tp_dlen);
		for (p=&prtp->tp_dat[0]; p < pend; ++p) {
			if (p == &rpbuf[offset] ||
			    p == &rpbuf[offset+1])
				continue;
			if (*p != 0xa5) {
				tst_resm(TFAIL, "IPV6_CHECKSUM corrupt data "
					"0x%02x != 0xa5 at offset %d in packet",
					*p, p - rpbuf);
				rv = 0;
				break;
			}
		}
		if (rv == 0)
			continue;
		ph.ph_len = htonl(xlen);
		cs = csum(0, (unsigned char *)&ph, sizeof(ph));
		cs = csum(~cs, rpbuf, xlen);
		if (!csum(0, rpbuf, xlen)) {
			tst_resm(TFAIL, "IPV6_CHECKSUM offset %d len %d (bad "
				"checksum)", offset, len);
			continue;
		}
		tst_resm(TPASS, "IPV6_CHECKSUM offset %d len %d", offset, len);
	}
	return 0;
}

static int listen_fd, connect_fd;
sem_t ilsem;

void *
ilistener(void *arg)
{
	connect_fd = accept(listen_fd, 0, 0);
	close(listen_fd);
	sem_post(&ilsem);
	return NULL;
}

int
isocketpair(int pf, int type, int proto, int fd[2])
{
	pthread_t	thid;
	struct sockaddr_in sin4;
	socklen_t namelen;

/* restrict to PF_INET for now */
	if (pf != PF_INET) {
		errno = EOPNOTSUPP;
		return -1;
	}
	sem_init(&ilsem, 0, 0);
	listen_fd = socket(pf, type, proto);
	if (listen_fd < 0) {
		perror("socket");
		return -1;
	}
	memset(&sin4, 0, sizeof(sin4));
	if (bind(listen_fd, (struct sockaddr *)&sin4, sizeof(sin4)) < 0) {
		perror("bind");
		return -1;
	}
	if (listen(listen_fd, 10) < 0) {
		perror("listen");
		return -1;
	}
	namelen = sizeof(sin4);
	if (getsockname(listen_fd, (struct sockaddr *)&sin4, &namelen) < 0) {
		perror("getsockname");
		return -1;
	}
	if (pthread_create(&thid, 0, ilistener, 0) < 0) {
		perror("pthread_create");
		return -1;
	}

	fd[0] = socket(pf, type, proto);
	if (fd[0] < 0) {
		perror("socket");
		return -1;
	}
	sin4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (connect(fd[0], (struct sockaddr *)&sin4, sizeof(sin4)) < 0) {
		perror("connect");
		return -1;
	}
	sem_wait(&ilsem);
	fd[1] = connect_fd;
	sem_destroy(&ilsem);
	return 0;
}

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif /* MAX */

int
csum_test(char *rhost)
{
	fd_set rset, rset_save;
	int csd[2];	/* control sockets */
	int sd, nfds, maxfd, cc;
	struct timeval tv;

/* rhost == loopback, for now */
	if (strcmp(rhost, "::1")) {
		tst_resm(TBROK, "invalid rhost \"%s\"", rhost);
		return -1;
	}
	if (isocketpair(PF_INET, SOCK_STREAM, 0, csd) < 0) {
		tst_resm(TBROK, "socketpair: %s", strerror(errno));
		return -1;
	}
	sd = socket(PF_INET6, SOCK_RAW, NH_TEST);
	if (sd < 0) {
		int saved_errno = errno;

		if (errno == EPERM && geteuid())
			tst_resm(TBROK, "IPV6_CHECKSUM tests must run as root");
		else
			tst_resm(TBROK, "All IPv6_CHECKSUM tests broken: "
				"socket: %s", strerror(saved_errno));
		return -1;
	}
	FD_ZERO(&rset_save);
	FD_SET(sd, &rset_save);
	FD_SET(csd[1], &rset_save);
	memcpy(&rset, &rset_save, sizeof(rset));
	maxfd = MAX(sd, csd[1]);

	/* server socket set; now start the client */
	switch (fork()) {
	case 0:	/* child */
		close(csd[0]);
		break;
	case -1:
		tst_resm(TBROK, "can't fork rserver");
		return -1;
	default: /* parent */
		close(sd);
		close(csd[1]);
		return client(pid, csd[0]);
	}

	tv.tv_sec = READ_TIMEOUT;
	tv.tv_usec = 0;
	while ((nfds = select(maxfd+1, &rset, 0, 0, &tv)) >= 0) {
		if (nfds < 0) {
			if (errno == EINTR)
				continue;
			exit(0);
		} else if (nfds == 0) {
			fprintf(stderr, "server read timed out");
			return -1;
		}
		if (FD_ISSET(sd, &rset)) {
			static char packet[2048];

			cc = recv(sd, packet, sizeof(packet),0);
			if (cc < 0) {
				perror("server recvtprot");
				exit(1);
			}
			if (cc == 0)
				exit(0);
			if (write(csd[1], packet, cc) < 0) {
				perror("server write UNIX socket");
				exit(0);
			}
		}
		if (FD_ISSET(csd[1], &rset)) {
			char buf[2048];

			cc = read(csd[1], buf, sizeof(buf));
			if (cc == 0) {
				exit(0);
			}
			if (cc < 0) {
				perror("server read");
				exit(1);
			}
			/* handle commands here, if any added later */
		}
		memcpy(&rset, &rset_save, sizeof(rset));
		tv.tv_sec = READ_TIMEOUT;
		tv.tv_usec = 0;
	}
	return 0;
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

int TST_TOTAL = PTCOUNT + CSCOUNT;