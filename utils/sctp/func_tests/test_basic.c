/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2001, 2003
 * Copyright (c) 1999-2000 Cisco, Inc.
 * Copyright (c) 1999-2001 Motorola, Inc.
 * Copyright (c) 2001 Intel Corp.
 * Copyright (c) 2001 Nokia, Inc.
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
 *    Hui Huang <hui.huang@nokia.com>
 *    Jon Grimm <jgrimm@us.ibm.com>
 *    Sridhar Samudrala <samudrala@us.ibm.com>
 */

/* This is a basic functional test for the SCTP kernel
 * implementation state machine.
 */ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 15;
int TST_CNT = 0;

int main(void)
{
        int sk1, sk2;
        sockaddr_storage_t loop1;
        sockaddr_storage_t loop2;
	sockaddr_storage_t msgname;
        struct iovec iov;
        struct msghdr inmessage;
	struct msghdr outmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sinfo;
        struct iovec out_iov;
        char *message = "hello, world!\n";
        char *telephone = "Watson, come here!  I need you!\n";
        char *telephone_resp = "I already brought your coffee...\n";
        int error, bytes_sent;
	int pf_class;
	uint32_t ppid;
	uint32_t stream;
	sctp_assoc_t associd1, associd2;
	struct sctp_assoc_change *sac;
	char *big_buffer;
	struct sockaddr *laddrs, *paddrs;
	int n_laddrs, n_paddrs, i;
	struct sockaddr *sa_addr;
	struct sockaddr_in *in_addr;
	struct sockaddr_in6 *in6_addr;
	void *addr_buf;

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered. 
	 */
	setvbuf(stdout, NULL, _IONBF, 0); 

	/* Set some basic values which depend on the address family. */
#if TEST_V6
	pf_class = PF_INET6;

        loop1.v6.sin6_family = AF_INET6;
        loop1.v6.sin6_addr = (struct in6_addr)SCTP_IN6ADDR_ANY_INIT;
        loop1.v6.sin6_port = htons(SCTP_TESTPORT_1);

        loop2.v6.sin6_family = AF_INET6;
        loop2.v6.sin6_addr = in6addr_loopback;
        loop2.v6.sin6_port = htons(SCTP_TESTPORT_2);
#else
	pf_class = PF_INET;

        loop1.v4.sin_family = AF_INET;
        loop1.v4.sin_addr.s_addr = INADDR_ANY;
        loop1.v4.sin_port = htons(SCTP_TESTPORT_1);

        loop2.v4.sin_family = AF_INET;
        loop2.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        loop2.v4.sin_port = htons(SCTP_TESTPORT_2);
#endif /* TEST_V6 */

        /* Create the two endpoints which will talk to each other.  */
        sk1 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
        sk2 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	tst_resm(TPASS, "socket");

        /* Bind these sockets to the test ports.  */
        test_bind(sk1, &loop1.sa, sizeof(loop1));
        test_bind(sk2, &loop2.sa, sizeof(loop2));

	tst_resm(TPASS, "bind");

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(sk1);
	test_enable_assoc_change(sk2);

        /* Initialize inmessage for all receives. */
	big_buffer = test_malloc(REALLY_BIG);
	memset(&inmessage, 0, sizeof(inmessage));	
        iov.iov_base = big_buffer;
        iov.iov_len = REALLY_BIG;
        inmessage.msg_iov = &iov;
        inmessage.msg_iovlen = 1;
        inmessage.msg_control = incmsg;
	inmessage.msg_name = &msgname;

        /* Try to read on socket 2.  This should fail since we are
	 * neither listening, nor established. 
	 */
        inmessage.msg_controllen = sizeof(incmsg);
        error = recvmsg(sk2, &inmessage, MSG_WAITALL);
        if (error > 0)
                tst_brkm(TBROK, tst_exit, "recvmsg on a socket neither"
			 "listening nor established error: %d", error);

	tst_resm(TPASS, "recvmsg on a socket neither listening nor "
		 "established");

       /* Mark sk2 as being able to accept new associations.  */
	error = test_listen(sk2, 1);
        
	tst_resm(TPASS, "listen");

        /* Send the first message.  This will create the association.  */
        outmessage.msg_name = &loop2;
        outmessage.msg_namelen = sizeof(loop2);
        outmessage.msg_iov = &out_iov;
        outmessage.msg_iovlen = 1;
        outmessage.msg_control = outcmsg;
        outmessage.msg_controllen = sizeof(outcmsg);
        outmessage.msg_flags = 0;
	cmsg = CMSG_FIRSTHDR(&outmessage);
	cmsg->cmsg_level = IPPROTO_SCTP;
	cmsg->cmsg_type = SCTP_SNDRCV;
	cmsg->cmsg_len = CMSG_LEN(sizeof(struct sctp_sndrcvinfo));
	outmessage.msg_controllen = cmsg->cmsg_len;
	sinfo = (struct sctp_sndrcvinfo *)CMSG_DATA(cmsg);
	memset(sinfo, 0x00, sizeof(struct sctp_sndrcvinfo));
	ppid = rand(); /* Choose an arbitrary value. */
	stream = 1; 
	sinfo->sinfo_ppid = ppid;
	sinfo->sinfo_stream = stream;
        outmessage.msg_iov->iov_base = message;
        outmessage.msg_iov->iov_len = strlen(message) + 1;
        test_sendmsg(sk1, &outmessage, 0, strlen(message)+1);
        
	tst_resm(TPASS, "sendmsg with a valid msg_name");

        /* Get the communication up message on sk2.  */
        inmessage.msg_controllen = sizeof(incmsg);
	inmessage.msg_namelen = sizeof(msgname);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
#if TEST_V6

	if (inmessage.msg_namelen != sizeof(struct sockaddr_in6)) {
		DUMP_CORE;
	}
	if (msgname.v6.sin6_port != htons(SCTP_TESTPORT_1)) {
		DUMP_CORE;
	}

	if (msgname.v6.sin6_family != AF_INET6) {
		DUMP_CORE;
	}

	if (memcmp(&msgname.v6.sin6_addr, &in6addr_loopback, 
		   sizeof(msgname.v6.sin6_addr))) {
		DUMP_CORE;
	}
#else 
	if (inmessage.msg_namelen != sizeof(struct sockaddr_in)) {
		DUMP_CORE;
	}
	if (msgname.v4.sin_port != htons(SCTP_TESTPORT_1)) {
		DUMP_CORE;
	}

	if (msgname.v4.sin_family != AF_INET) {
		DUMP_CORE;
	}
	if (msgname.v4.sin_addr.s_addr != SCTP_IP_LOOPBACK) {
		DUMP_CORE;
	}
#endif
	sac = (struct sctp_assoc_change *)iov.iov_base;
	associd2 = sac->sac_assoc_id;

        /* Get the communication up message on sk1.  */
	iov.iov_base = big_buffer;
        iov.iov_len = REALLY_BIG;
        inmessage.msg_control = incmsg;
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk1, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error, 
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	associd1 = sac->sac_assoc_id;

	tst_resm(TPASS, "recvmsg COMM_UP notifications");

        /* Get the first message which was sent.  */
        inmessage.msg_controllen = sizeof(incmsg);
	inmessage.msg_namelen = sizeof(msgname);
	memset(&msgname, 0, sizeof(msgname));
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(message) + 1,
			    MSG_EOR, stream, ppid);
#if TEST_V6

	if (inmessage.msg_namelen != sizeof(struct sockaddr_in6)) {
		DUMP_CORE;
	}
	if (msgname.v6.sin6_port != htons(SCTP_TESTPORT_1)) {
		DUMP_CORE;
	}

	if (msgname.v6.sin6_family != AF_INET6) {
		DUMP_CORE;
	}

	if (memcmp(&msgname.v6.sin6_addr, &in6addr_loopback, 
		   sizeof(msgname.v6.sin6_addr))) {
		DUMP_CORE;
	}
#else 
	if (inmessage.msg_namelen != sizeof(struct sockaddr_in)) {
		DUMP_CORE;
	}
	if (msgname.v4.sin_port != htons(SCTP_TESTPORT_1)) {
		DUMP_CORE;
	}
	if (msgname.v4.sin_family != AF_INET) {
		DUMP_CORE;
	}
	if (msgname.v4.sin_addr.s_addr != SCTP_IP_LOOPBACK) {
		DUMP_CORE;
	}
#endif

	/* Try to send a message with NULL msg_name and associd, should fail */
        outmessage.msg_controllen = sizeof(outcmsg);
        outmessage.msg_flags = 0;
	cmsg = CMSG_FIRSTHDR(&outmessage);
	cmsg->cmsg_level = IPPROTO_SCTP;
	cmsg->cmsg_type = SCTP_SNDRCV;
	cmsg->cmsg_len = CMSG_LEN(sizeof(struct sctp_sndrcvinfo));
	outmessage.msg_controllen = cmsg->cmsg_len;
	sinfo = (struct sctp_sndrcvinfo *)CMSG_DATA(cmsg);
	memset(sinfo, 0x00, sizeof(struct sctp_sndrcvinfo));
	ppid++;
	stream++;
	sinfo->sinfo_ppid = ppid;
	sinfo->sinfo_stream = stream;
	outmessage.msg_iov->iov_base = telephone;
        outmessage.msg_iov->iov_len = strlen(telephone) + 1;
	outmessage.msg_name = NULL;
	outmessage.msg_namelen = 0;
	bytes_sent = sendmsg(sk1, &outmessage, MSG_NOSIGNAL);
	if ((bytes_sent > 0) || (EPIPE != errno))
		tst_brkm(TBROK, tst_exit, "sendmsg with NULL associd and "
			 "NULL msg_name error:%d errno:%d", error, errno);

	tst_resm(TPASS, "sendmsg with NULL associd and NULL msg_name");

	/* Fill in a incorrect assoc_id, which should cause an error. */
	sinfo->sinfo_assoc_id = associd2;
	bytes_sent = sendmsg(sk1, &outmessage, MSG_NOSIGNAL);
	if ((bytes_sent > 0) || (EPIPE != errno))
		tst_brkm(TBROK, tst_exit, "sendmsg with incorrect associd "
			 "error:%d errno:%d", error, errno);

	tst_resm(TPASS, "sendmsg with incorrect associd");

	/* Fill in a correct assoc_id and get back to the normal testing. */
	sinfo->sinfo_assoc_id = associd1;
        /* Send two more messages, to cause a second SACK.  */
	test_sendmsg(sk1, &outmessage, 0, strlen(telephone)+1);

	outmessage.msg_name = &loop2;
	outmessage.msg_namelen = sizeof(loop2);
	outmessage.msg_iov->iov_base = telephone_resp;
        outmessage.msg_iov->iov_len = strlen(telephone_resp) + 1;
	test_sendmsg(sk1, &outmessage, 0, strlen(telephone_resp)+1);

	tst_resm(TPASS, "sendmsg with valid associd");

        /* Get those two messages.  */
	inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(telephone) + 1,
			    MSG_EOR, stream, ppid);

	inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(telephone_resp) + 1,
			    MSG_EOR, stream, ppid);
       
	tst_resm(TPASS, "recvmsg");

	n_laddrs = sctp_getladdrs(sk1, associd1, &laddrs); 
	if (n_laddrs <= 0)
                tst_brkm(TBROK, tst_exit, "sctp_getladdrs: %s",
			 strerror(errno));

	tst_resm(TPASS, "sctp_getladdrs");

	addr_buf = (void *)laddrs;
	for (i = 0; i < n_laddrs; i++) {
		sa_addr = (struct sockaddr *)addr_buf;
		if (AF_INET == sa_addr->sa_family) {
			in_addr = (struct sockaddr_in *)sa_addr;
			tst_resm(TINFO, "LOCAL ADDR %d.%d.%d.%d PORT %d",
				 NIPQUAD(in_addr->sin_addr),
				 ntohs(in_addr->sin_port));
			addr_buf += sizeof(struct sockaddr_in);
		} else {
			in6_addr = (struct sockaddr_in6 *)sa_addr;
			tst_resm(TINFO,
		 "LOCAL ADDR %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x PORT %d",
			       NIP6(in6_addr->sin6_addr),
			       ntohs(in6_addr->sin6_port));
			addr_buf += sizeof(struct sockaddr_in6);
		}
	}

	sctp_freeladdrs(laddrs);

	tst_resm(TPASS, "sctp_freeladdrs");

	n_paddrs = sctp_getpaddrs(sk1, associd1, &paddrs); 
	if (n_paddrs <= 0)
                tst_brkm(TBROK, tst_exit, "sctp_getpaddrs: %s",
			 strerror(errno));

	tst_resm(TPASS, "sctp_getpaddrs");

	addr_buf = (void *)paddrs;
	for (i = 0; i < n_paddrs; i++) {
		sa_addr = (struct sockaddr *)addr_buf;
		if (AF_INET == sa_addr->sa_family) {
			in_addr = (struct sockaddr_in *)sa_addr;
			tst_resm(TINFO, "PEER ADDR %d.%d.%d.%d PORT %d",
				 NIPQUAD(in_addr->sin_addr),
				 ntohs(in_addr->sin_port));
			addr_buf += sizeof(struct sockaddr_in);
		} else {
			in6_addr = (struct sockaddr_in6 *)sa_addr;
			tst_resm(TINFO,
		 "PEER ADDR %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x PORT %d",
			       NIP6(in6_addr->sin6_addr),
			       ntohs(in6_addr->sin6_port));
			addr_buf += sizeof(struct sockaddr_in6);
		}
	}

	sctp_freepaddrs(paddrs);

	tst_resm(TPASS, "sctp_freepaddrs");

        /* Shut down the link.  */
        close(sk1);

        /* Get the shutdown complete notification. */
	inmessage.msg_controllen = sizeof(incmsg);
	inmessage.msg_namelen = sizeof(msgname);
	memset(&msgname, 0, sizeof(msgname));
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);	
#if TEST_V6

	if (inmessage.msg_namelen != sizeof(struct sockaddr_in6)) {
		DUMP_CORE;
	}
	if (msgname.v6.sin6_port != htons(SCTP_TESTPORT_1)) {
		DUMP_CORE;
	}

	if (msgname.v6.sin6_family != AF_INET6) {
		DUMP_CORE;
	}

	if (memcmp(&msgname.v6.sin6_addr, &in6addr_loopback, 
		   sizeof(msgname.v6.sin6_addr))) {
		DUMP_CORE;
	}
#else 
	if (inmessage.msg_namelen != sizeof(struct sockaddr_in)) {
		DUMP_CORE;
	}
	if (msgname.v4.sin_port != htons(SCTP_TESTPORT_1)) {
		DUMP_CORE;
	}

	if (msgname.v4.sin_family != AF_INET) {
		DUMP_CORE;
	}
	if (msgname.v4.sin_addr.s_addr != SCTP_IP_LOOPBACK) {
		DUMP_CORE;
	}
#endif
				
	tst_resm(TPASS, "recvmsg SHUTDOWN_COMP notification");

        close(sk2);

        /* Indicate successful completion.  */
       	return 0; 
}
