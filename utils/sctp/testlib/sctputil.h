/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2001, 2003
 * Copyright (c) 1999-2000 Cisco, Inc.
 * Copyright (c) 1999-2001 Motorola, Inc.
 * Copyright (c) 2001 Intel Corp.
 * Copyright (c) 2001 Nokia, Inc.
 * Copyright (c) 2001 La Monte H.P. Yarroll
 *
 * The SCTP implementation is free software;
 * you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * The SCTP implementation is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 ************************
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Please send any bug reports or fixes you make to the
 * email address(es):
 *    lksctp developers <lksctp-developers@lists.sourceforge.net>
 *
 * Or submit a bug report through the following website:
 *    http://www.sf.net/projects/lksctp
 *
 * Any bugs reported to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release.
 *
 * Written or modified by:
 *    La Monte H.P. Yarroll <piggy@acm.org>
 *    Karl Knutson <karl@athena.chicago.il.us>
 *    Randall Stewart <randall@stewart.chicago.il.us>
 *    Ken Morneau <kmorneau@cisco.com>
 *    Qiaobing Xie <qxie1@motorola.com>
 *    Daisy Chang <daisyc@us.ibm.com>
 *    Jon Grimm <jgrimm@us.ibm.com>
 *    Sridhar Samudrala <samudrala@us.ibm.com>
 *    Hui Huang <hui.huang@nokia.com>
 */

#ifndef __sctputil_h__
#define __sctputil_h__

#ifdef LTP
#include <test.h>
#include <tso_usctest.h>
#endif

#include <string.h>

typedef union {
	struct sockaddr_in v4;
	struct sockaddr_in6 v6;
	struct sockaddr sa;
} sockaddr_storage_t;


#define REALLY_BIG 65536

/* Literal defines.  */
#ifdef PROT_SOCK
#define SCTP_TESTPORT_1 PROT_SOCK
#else
#define SCTP_TESTPORT_1 1024
#endif
#define SCTP_TESTPORT_2 (SCTP_TESTPORT_1+1)

#define SCTP_IP_BCAST  	htonl(0xffffffff)
#define SCTP_IP_LOOPBACK  htonl(0x7f000001)

/* These are stolen from <netinet/in.h>.  */
#define SCTP_IN6ADDR_ANY_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
#define SCTP_IN6ADDR_LOOPBACK_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }

/* Display an IPv4 address in readable format.  */
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]

/* Display an IPv6 address in readable format.  */
#define NIP6(addr) \
        ntohs((addr).s6_addr16[0]), \
        ntohs((addr).s6_addr16[1]), \
        ntohs((addr).s6_addr16[2]), \
        ntohs((addr).s6_addr16[3]), \
        ntohs((addr).s6_addr16[4]), \
        ntohs((addr).s6_addr16[5]), \
        ntohs((addr).s6_addr16[6]), \
        ntohs((addr).s6_addr16[7])

#define DUMP_CORE { 					 \
	char *diediedie = 0;				 \
	printf("DUMP_CORE %s: %d\n", __FILE__, __LINE__);\
	*diediedie = 0;					 \
}

#ifndef LTP
enum {
	TPASS,
	TINFO,
};

extern char *TCID;
extern int TST_TOTAL;
extern int TST_CNT;

#define tst_brkm(a1, a2, whatever...) \
	{ \
		printf("%s %2d BROK : ", TCID, ++TST_CNT); \
		printf(whatever); \
		printf("\n"); \
		DUMP_CORE \
	}
#define tst_resm(a1, whatever...) \
	{ \
		printf("%s %2d %s : ", TCID, \
			 (a1 == TPASS)?++TST_CNT:0, \
			 (a1 == TPASS)?"PASS":"INFO"); \
		printf(whatever); \
		printf("\n"); \
	}
#endif

static inline int test_socket(int domain, int type, int protocol)
{
	int sk = socket(domain, type, protocol);

	if (sk == -1) {
		if (errno == EAFNOSUPPORT)
			tst_brkm(TCONF | TERRNO, tst_exit, "socket(%i, %i, %i) not supported", domain,
					 type, protocol);

		tst_brkm(TBROK | TERRNO, tst_exit, "socket()");
	}

	return sk;
}

static inline int test_bind(int sk, struct sockaddr *addr, socklen_t addrlen)
{
	int error = bind(sk, addr, addrlen);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "bind()");

	return error;
}

static inline int test_bindx_add(int sk, struct sockaddr *addr, int count)
{
	int error = sctp_bindx(sk, addr, count, SCTP_BINDX_ADD_ADDR);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "sctp_bindx()");

	return error;
}

static inline int test_listen(int sk, int backlog)
{
	int error = listen(sk, backlog);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "listen()");

	return error;
}

static inline int test_connect(int sk, struct sockaddr *addr, socklen_t addrlen)
{
	int error = connect(sk, addr, addrlen);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "connect()");

	return error;
}

static inline int test_connectx(int sk, struct sockaddr *addr, int count)
{
	int error = sctp_connectx(sk, addr, count, NULL);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "connectx()");

	return error;
}

static inline int test_accept(int sk, struct sockaddr *addr, socklen_t *addrlen)
{
	int error = accept(sk, addr, addrlen);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "accept()");

	return error;
}

static inline int test_send(int sk, const void *msg, size_t len, int flags)
{
	int error = send(sk, msg, len, flags);

	if ((long)len != error)
		tst_brkm(TBROK | TERRNO, tst_exit, "send(): error: %d", error);

	return error;
}

static inline int test_sendto(int sk, const void *msg, size_t len, int flags,
			      const struct sockaddr *to, socklen_t tolen)
{
	int error = sendto(sk, msg, len, flags, to, tolen);

	if ((long)len != error)
		tst_brkm(TBROK | TERRNO, tst_exit, "sendto(): error: %d", error);

	return error;
}

static inline int test_sendmsg(int sk, const struct msghdr *msg, int flags,
			       int msglen)
{
	int error = sendmsg(sk, msg, flags);

	if (msglen != error)
		tst_brkm(TBROK | TERRNO, tst_exit, "sendmsg(): error: %d", error);

	return error;
}

static inline int test_recv(int sk, void *buf, size_t len, int flags)
{
	int error = recv(sk, buf, len, flags);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "recv()");

	return error;
}

static inline int test_recvmsg(int sk, struct msghdr *msg, int flags)
{
	int error = recvmsg(sk, msg, flags);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "recvmsg()");

	return error;
}

static inline int test_shutdown(int sk, int how)
{
	int error = shutdown(sk, how);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "shutdown()");

	return error;
}

static inline int test_getsockopt(int sk, int optname, void *optval,
				  socklen_t *optlen)
{
	int error = getsockopt(sk, SOL_SCTP, optname, optval, optlen);

	if (error)
		tst_brkm(TBROK, tst_exit, "getsockopt(%d): %s", optname,
			 strerror(errno));
	return error;
}

static inline int test_setsockopt(int sk, int optname, const void *optval,
				  socklen_t optlen)
{
	int error = setsockopt(sk, SOL_SCTP, optname, optval, optlen);

	if (error)
		tst_brkm(TBROK, tst_exit, "setsockopt(%d): %s", optname,
			 strerror(errno));

	return error;
}

static inline int test_sctp_peeloff(int sk, sctp_assoc_t assoc_id)
{
	int error = sctp_peeloff(sk, assoc_id);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "sctp_peeloff()");

	return error;
}

static inline int test_sctp_sendmsg(int s, const void *msg, size_t len,
				    struct sockaddr *to, socklen_t tolen,
				    uint32_t ppid, uint32_t flags,
				    uint16_t stream_no, uint32_t timetolive,
				    uint32_t context)
{
	int error = sctp_sendmsg(s, msg, len, to, tolen, ppid, flags, stream_no,
				 timetolive, context);

	if (error != (long)len)
		tst_brkm(TBROK, tst_exit, "sctp_sendmsg: error:%d errno:%d",
			 error, errno);

	return error;
}

static inline int test_sctp_send(int s, const void *msg, size_t len,
				 const struct sctp_sndrcvinfo *sinfo,
				 int flags)
{
	int error = sctp_send(s, msg, len, sinfo, flags);

	if (error != (long)len)
		tst_brkm(TBROK, tst_exit, "sctp_send: error:%d errno:%d",
			 error, errno);

	return error;
}

static inline int test_sctp_recvmsg(int sk, void *msg, size_t len,
				    struct sockaddr *from, socklen_t *fromlen,
				    struct sctp_sndrcvinfo *sinfo,
				    int *msg_flags)
{
	int error = sctp_recvmsg(sk, msg, len, from, fromlen, sinfo, msg_flags);

	if (error == -1)
		tst_brkm(TBROK | TERRNO, tst_exit, "sctp_recvmsg()");

	return error;
}

static inline void *test_malloc(size_t size)
{
	void *buf = malloc(size);

	if (NULL == buf)
		tst_brkm(TBROK, tst_exit, "malloc failed");

	return buf;
}

void test_check_msg_notification(struct msghdr *, int, int, uint16_t, uint32_t);
void test_check_buf_notification(void *, int, int, int, uint16_t, uint32_t);
void test_check_msg_data(struct msghdr *, int, int, int, uint16_t, uint32_t);
void test_check_buf_data(void *, int, int, struct sctp_sndrcvinfo *, int, int,
			 uint16_t, uint32_t);
void *test_build_msg(int);
void test_enable_assoc_change(int);
void test_print_message(int sk, struct msghdr *msg, size_t msg_len);
int test_peer_addr(int sk, sctp_assoc_t asoc, sockaddr_storage_t *peers, int count);

#endif /* __sctputil_h__ */
