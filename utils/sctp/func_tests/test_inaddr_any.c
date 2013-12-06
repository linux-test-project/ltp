/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2002, 2003
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
 *    Jon Grimm <jgrimm@us.ibm.com>
 *    Sridhar Samudrala <sri@us.ibm.com>
 *    Daisy Chang <daisyc@us.ibm.com>
 */

/* This is a functional test to verify binding a socket with INADDRY_ANY
 * address and send messages.
 */ 

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 2;
int TST_CNT = 0;

int
main(void)
{
        int sk1, sk2;
        sockaddr_storage_t loop;
        sockaddr_storage_t anyaddr;
        struct msghdr outmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sinfo;
        struct iovec out_iov;
        struct iovec iov;
        struct msghdr inmessage;
        char *message = "hello, world!\n";
        char *telephone = "Watson, come here!  I need you!\n";
        char *telephone_resp = "I already brought your coffee...\n";
        int error;
	int pf_class;
	uint32_t ppid;
	uint32_t stream;
	socklen_t namelen;

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered. 
	 */
	setvbuf(stdout, NULL, _IONBF, 0); 

	/* Set some basic values which depend on the address family. */
#if TEST_V6
	pf_class = PF_INET6;

        loop.v6.sin6_family = AF_INET6;
        loop.v6.sin6_addr = (struct in6_addr)SCTP_IN6ADDR_LOOPBACK_INIT;
        loop.v6.sin6_port = 0;

        anyaddr.v6.sin6_family = AF_INET6;
        anyaddr.v6.sin6_addr = (struct in6_addr)SCTP_IN6ADDR_ANY_INIT;
        anyaddr.v6.sin6_port = 0;
#else
	pf_class = PF_INET;

        loop.v4.sin_family = AF_INET;
        loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        loop.v4.sin_port = 0;

        anyaddr.v4.sin_family = AF_INET;
        anyaddr.v4.sin_addr.s_addr = INADDR_ANY;
        anyaddr.v4.sin_port = 0;
#endif /* TEST_V6 */

        /* Create the two endpoints which will talk to each other.  */
        sk1 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
        sk2 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(sk1);
	test_enable_assoc_change(sk2);

        /* Bind these sockets to the test ports.  */
        test_bind(sk1, &loop.sa, sizeof(loop));
        test_bind(sk2, &anyaddr.sa, sizeof(anyaddr));

	tst_resm(TPASS, "bind INADDR_ANY address");

 	/* Mark sk2 as being able to accept new associations */
	test_listen(sk2, 1);

	/* Now use getsockaname() to retrieve the ephmeral ports. */
	namelen = sizeof(loop);
	error = getsockname(sk1, &loop.sa, &namelen);
	if (error != 0)
		tst_brkm(TBROK, tst_exit, "getsockname: %s", strerror(errno));

	namelen = sizeof(anyaddr);
	error = getsockname(sk2, &anyaddr.sa, &namelen);
	if (error != 0)
		tst_brkm(TBROK, tst_exit, "getsockname: %s", strerror(errno));
        
#if TEST_V6
	loop.v6.sin6_port = anyaddr.v6.sin6_port;
#else
        loop.v4.sin_port = anyaddr.v4.sin_port;
#endif

        /* Send the first message.  This will create the association.  */
        outmessage.msg_name = &loop;
        outmessage.msg_namelen = sizeof(loop);
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

	/* Initialize inmessage for all receives. */
        memset(&inmessage, 0, sizeof(inmessage));
        iov.iov_base = test_malloc(REALLY_BIG);
        iov.iov_len = REALLY_BIG;
        inmessage.msg_iov = &iov;
        inmessage.msg_iovlen = 1;
        inmessage.msg_control = incmsg;

        /* Get the communication up message on sk2.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	

        /* Get the communication up message on sk1.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk1, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	

        /* Get the first message which was sent.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(message) + 1, 
			    MSG_EOR, stream, ppid);

       /* Send 2 messages.  */
        outmessage.msg_name = &loop;
        outmessage.msg_namelen = sizeof(loop);
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
	test_sendmsg(sk1, &outmessage, 0, strlen(telephone)+1);

	outmessage.msg_iov->iov_base = telephone_resp;
        outmessage.msg_iov->iov_len = strlen(telephone_resp) + 1;
	test_sendmsg(sk1, &outmessage, 0, strlen(telephone_resp)+1);
        
        /* Get those two messages.  */
	inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(telephone) + 1, 
			    MSG_EOR, stream, ppid);

	inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(telephone_resp) + 1, 
			    MSG_EOR, stream, ppid);
        
        /* Shut down the link.  */
        close(sk1);

        /* Get the shutdown complete notification. */
	inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);

        close(sk2);

	tst_resm(TPASS, "send msgs from a socket with INADDR_ANY bind address");

        /* Indicate successful completion.  */
        return 0;
}
