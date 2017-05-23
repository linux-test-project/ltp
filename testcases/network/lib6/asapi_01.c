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
#include <netdb.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "asapi_04";

static pid_t pid;

static struct {
	char *prt_name;
	int prt_value;
} ptab[] = {
	{"hopopt", 0},
	{"ipv6", 41},
	{"ipv6-route", 43},
	{"ipv6-frag", 44},
	{"esp", 50},
	{"ah", 51},
	{"ipv6-icmp", 58},
	{"ipv6-nonxt", 59},
	{"ipv6-opts", 60},
};

#define PTCOUNT		ARRAY_SIZE(ptab)
#define READ_TIMEOUT	5

static void do_tests(void);
static void setup(void);
static void csum_test(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, 0, 0);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc)
		do_tests();

	tst_exit();
}

static void do_tests(void)
{
	unsigned int i;

/* RFC 3542, Section 2.3 */
#ifndef IN6_ARE_ADDR_EQUAL
	tst_resm(TCONF, "IN6_ARE_ADDR_EQUAL not present");
#else /* IN6_ARE_ADDR_EQUAL */
	/*
	 * set each bit in an address and check for unequal; then set
	 * in the second address and check for equal. Covers all bits, all
	 * combinations.
	 */
	struct in6_addr a1, a2;
	int word, bit;
	int rv = 1;

	memset(&a1, 0, sizeof(a1));
	memset(&a2, 0, sizeof(a2));

	rv = IN6_ARE_ADDR_EQUAL(&a1, &a2);

	for (word = 0; word < 4; ++word) {
		for (bit = 0; bit < 32; ++bit) {
			uint32_t newbit = 1U << bit;

			a1.s6_addr32[word] |= newbit;
			rv &= !IN6_ARE_ADDR_EQUAL(&a1, &a2);
			a2.s6_addr32[word] |= newbit;
			rv &= IN6_ARE_ADDR_EQUAL(&a1, &a2);
		}
	}

	tst_resm(rv ? TPASS : TFAIL, "IN6_ARE_ADDR_EQUAL");
#endif /* IN6_ARE_ADDR_EQUAL */

/* RFC 3542, Section 2.4 */
	for (i = 0; i < PTCOUNT; ++i) {
		struct protoent *pe;
		int pass;

		pe = getprotobyname(ptab[i].prt_name);
		pass = pe && pe->p_proto == ptab[i].prt_value;
		tst_resm(pass ? TPASS : TFAIL, "\"%s\" protocols entry",
			 ptab[i].prt_name);
	}
/* RFC 3542, Section 3.1 */
	csum_test();
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
	int tp_pid;		/* sender PID */
	int tp_seq;		/* sequence # */
	int tp_offset;		/* offset of cksum */
	int tp_dlen;		/* tp_dat length */
	unsigned char tp_dat[0];	/* user data */
};

static unsigned char tpbuf[sizeof(struct tprot) + 2048];
static unsigned char rpbuf[sizeof(struct tprot) + 2048];

static struct csent {
	int cs_offset;
	int cs_dlen;
	int cs_setresult;	/* setsockopt expected result */
	int cs_seterrno;	/* setsockopt expected errno */
	int cs_sndresult;	/* send expected result */
	int cs_snderrno;	/* send expected errno */
} cstab[] = {
	{0, 5, 0, 0, 0, 0},
	{6, 30, 0, 0, 0, 0},
	{3, 20, -1, EINVAL, -1, -1},	/* non-aligned offset */
	{4, 5, 0, 0, -1, EINVAL},	/* not enough space */
	{50, 5, 0, 0, -1, EINVAL},	/* outside of packet */
	{22, 30, 0, 0, 0, 0},
	{2000, 2004, 0, 0, 0, 0},	/* in a fragment (over Ethernet) */
};

#define CSCOUNT	ARRAY_SIZE(cstab)

int TST_TOTAL = PTCOUNT + CSCOUNT;

static int recvtprot(int sd, unsigned char *packet)
{
	struct tprot *tpt;
	int cc;
	unsigned int total, expected;
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
		cc = recv(sd, &packet[total], expected - total, 0);
		if (cc >= 0) {
			total += cc;
			if (!gothead && total >= sizeof(struct tprot)) {
				gothead = 1;
				expected += ntohl(tpt->tp_dlen);
			}
		} else {
			break;
		}
	}
	if (cc < 0)
		return cc;
	return total;
}

static unsigned short csum(unsigned short partial, unsigned char *packet,
			   int len)
{
	unsigned long sum = partial;
	unsigned short *ps;
	int i;

	ps = (unsigned short *)packet;
	for (i = 0; i < len / 2; ++i)
		sum += *ps++;
	if (len & 1)
		sum += htons(packet[len - 1] << 8);
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ~sum;
}

static struct ph {
	struct in6_addr ph_sa;
	struct in6_addr ph_da;
	uint32_t ph_len;
	uint8_t ph_mbz[3];
	uint8_t ph_nh;
} ph;

static int client(int sfd)
{
	struct tprot *pttp = (struct tprot *)tpbuf;
	struct tprot *prtp = (struct tprot *)rpbuf;
	struct sockaddr_in6 rsin6;
	static int seq;
	unsigned int i;
	int sd, cc, cs;

	memset(&rsin6, 0, sizeof(rsin6));
	rsin6.sin6_family = AF_INET6;
	rsin6.sin6_addr = in6addr_loopback;

	memset(&ph, 0, sizeof(ph));
	ph.ph_sa = rsin6.sin6_addr;
	ph.ph_da = rsin6.sin6_addr;
	ph.ph_nh = NH_TEST;

	sd = SAFE_SOCKET(NULL, PF_INET6, SOCK_RAW, NH_TEST);

	for (i = 0; i < CSCOUNT; ++i) {
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
			tst_resm(TFAIL | TTERRNO,
				 "IPV6_CHECKSUM offset %d len %d "
				 "- result %ld != %d", offset, len, TEST_RETURN,
				 cstab[i].cs_setresult);
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
			tst_resm(TFAIL | TTERRNO,
				 "IPV6_CHECKSUM offset %d len %d "
				 "- sndresult %ld != %d", offset, len,
				 TEST_RETURN, xlen);
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
		while ((cc = recvtprot(sfd, rpbuf))) {
			if (htonl(prtp->tp_pid) == (uint32_t)pid &&
			    htonl(prtp->tp_seq) == (uint32_t)seq)
				break;
		}
		rv = 1;
		pend = rpbuf + sizeof(struct tprot) + ntohl(prtp->tp_dlen);
		for (p = &prtp->tp_dat[0]; p < pend; ++p) {
			if (p == &rpbuf[offset] || p == &rpbuf[offset + 1])
				continue;
			if (*p != 0xa5) {
				tst_resm(TFAIL, "IPV6_CHECKSUM corrupt data "
					 "0x%02x != 0xa5, offset %zd in packet",
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

static void *ilistener(void *arg LTP_ATTRIBUTE_UNUSED)
{
	connect_fd = accept(listen_fd, 0, 0);
	close(listen_fd);
	return NULL;
}

static void isocketpair(int pf, int type, int proto, int fd[2])
{
	pthread_t thid;
	struct sockaddr_in sin4;
	socklen_t namelen;

	listen_fd = SAFE_SOCKET(NULL, pf, type, proto);

	memset(&sin4, 0, sizeof(sin4));

	SAFE_BIND(NULL, listen_fd, (struct sockaddr *)&sin4, sizeof(sin4));

	SAFE_LISTEN(NULL, listen_fd, 10);

	namelen = sizeof(sin4);
	SAFE_GETSOCKNAME(NULL, listen_fd, (struct sockaddr *)&sin4, &namelen);

	if (pthread_create(&thid, 0, ilistener, 0) < 0)
		tst_brkm(TBROK | TERRNO, NULL, "pthread_create error");

	fd[0] = SAFE_SOCKET(NULL, pf, type, proto);

	sin4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	SAFE_CONNECT(NULL, fd[0], (struct sockaddr *)&sin4, sizeof(sin4));

	pthread_join(thid, NULL);

	fd[1] = connect_fd;
}

static void csum_test(void)
{
	fd_set rset, rset_save;
	int csd[2];		/* control sockets */
	int sd, nfds, maxfd, cc;
	struct timeval tv;

	isocketpair(PF_INET, SOCK_STREAM, 0, csd);

	sd = SAFE_SOCKET(NULL, PF_INET6, SOCK_RAW, NH_TEST);

	FD_ZERO(&rset_save);
	FD_SET(sd, &rset_save);
	FD_SET(csd[1], &rset_save);
	memcpy(&rset, &rset_save, sizeof(rset));
	maxfd = MAX(sd, csd[1]);

	/* server socket set; now start the client */
	switch (fork()) {
	case 0:
		close(csd[0]);
		break;
	case -1:
		tst_brkm(TBROK, NULL, "can't fork rserver");
	default:
		close(sd);
		close(csd[1]);
		client(csd[0]);
		return;
	}

	tv.tv_sec = READ_TIMEOUT;
	tv.tv_usec = 0;
	while ((nfds = select(maxfd + 1, &rset, 0, 0, &tv)) >= 0) {
		if (nfds < 0) {
			if (errno == EINTR)
				continue;
			exit(0);
		} else if (nfds == 0) {
			fprintf(stderr, "server read timed out");
			return;
		}
		if (FD_ISSET(sd, &rset)) {
			static char packet[2048];

			cc = recv(sd, packet, sizeof(packet), 0);
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
			if (cc == 0)
				exit(0);
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
}

static void setup(void)
{
	TEST_PAUSE;

	pid = getpid();
}
