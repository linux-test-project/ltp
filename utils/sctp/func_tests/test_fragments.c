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
 *    Sridhar Samudrala <sri@us.ibm.com>
 */

/* This is a functional test to verify the data fragmentation, reassembly 
 * support and SCTP_DISABLE_FRAGMENTS socket option. 
 * The following tests are done in sequence.
 * - Verify SCTP_DISABLE_FRAGMENTS socket option by doing a setsockopt()
 *   followed by a getsockopt().
 * - Verify that a message size exceeding the association fragmentation
 *   point cannot be sent when fragmentation is disabled.
 * - Send and receive a set of messages that are bigger than the path mtu. 
 *   The different message sizes to be tested are specified in the array 
 *   msg_sizes[]. 
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
int TST_TOTAL = 4;
int TST_CNT = 0;

int msg_sizes[] = {1353, 2000, 5000, 10000, 20000, 32768};

int
main(int argc, char *argv[])
{
        int sk1, sk2;
        sockaddr_storage_t loop1;
        sockaddr_storage_t loop2;
        struct iovec iov;
        struct msghdr inmessage;
	struct msghdr outmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sinfo;
        struct iovec out_iov;
        int error, bytes_sent;
	int pf_class;
	uint32_t ppid;
	uint32_t stream;
	char *big_buffer;
	int msg_len, msg_cnt, i;
	void *msg_buf;
	int disable_frag;
	socklen_t optlen;

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered. 
	 */
	setvbuf(stdout, NULL, _IONBF, 0); 

	/* Set some basic values which depend on the address family. */
#if TEST_V6
	pf_class = PF_INET6;

        loop1.v6.sin6_family = AF_INET6;
        loop1.v6.sin6_addr = in6addr_loopback;
        loop1.v6.sin6_port = htons(SCTP_TESTPORT_1);

        loop2.v6.sin6_family = AF_INET6;
        loop2.v6.sin6_addr = in6addr_loopback;
        loop2.v6.sin6_port = htons(SCTP_TESTPORT_2);
#else
	pf_class = PF_INET;

        loop1.v4.sin_family = AF_INET;
        loop1.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        loop1.v4.sin_port = htons(SCTP_TESTPORT_1);

        loop2.v4.sin_family = AF_INET;
        loop2.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        loop2.v4.sin_port = htons(SCTP_TESTPORT_2);
#endif /* TEST_V6 */

        /* Create the two endpoints which will talk to each other.  */
        sk1 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
        sk2 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(sk1);
	test_enable_assoc_change(sk2);

        /* Bind these sockets to the test ports.  */
        test_bind(sk1, &loop1.sa, sizeof(loop1));
        test_bind(sk2, &loop2.sa, sizeof(loop2));

       /* Mark sk2 as being able to accept new associations.  */
	test_listen(sk2, 1);

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
	msg_len = 10;	
	msg_buf = test_build_msg(10);
        outmessage.msg_iov->iov_base = msg_buf;
        outmessage.msg_iov->iov_len = msg_len;
        test_sendmsg(sk1, &outmessage, 0, msg_len);
        

	/* Initialize inmessage for all receives. */
	big_buffer = test_malloc(REALLY_BIG);
        memset(&inmessage, 0, sizeof(inmessage));	
        iov.iov_base = big_buffer;
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
#if 0
	sac = (struct sctp_assoc_change *)iov.iov_base;
	associd2 = sac->sac_assoc_id;
#endif
        /* Get the communication up message on sk1.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk1, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
#if 0
	sac = (struct sctp_assoc_change *)iov.iov_base;
	associd1 = sac->sac_assoc_id;
#endif
        /* Get the first message which was sent.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, msg_len, MSG_EOR, stream, ppid);

	free(msg_buf);

	/* Disable fragmentation. */
	disable_frag = 1;
	test_setsockopt(sk1, SCTP_DISABLE_FRAGMENTS, &disable_frag,
			sizeof(disable_frag));

	tst_resm(TPASS, "setsockopt(SCTP_DISABLE_FRAGMENTS)");

	/* Do a getsockopt() and verify that fragmentation is disabled. */ 
	disable_frag = 0;
	optlen = sizeof(disable_frag);
	error = test_getsockopt(sk1, SCTP_DISABLE_FRAGMENTS, &disable_frag,
				&optlen);
	if ((error != 0) && (disable_frag != 1))
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DISABLE_FRAGMENTS) "
			 "error:%d errno:%d disable_frag:%d",
			 error, errno, disable_frag);

	tst_resm(TPASS, "getsockopt(SCTP_DISABLE_FRAGMENTS)");

	/* Try to send a messsage that exceeds association fragmentation point
	 * and verify that it fails.
	 */
	msg_len = 100000;
	msg_buf = test_build_msg(msg_len);
	outmessage.msg_iov->iov_base = msg_buf;
	outmessage.msg_iov->iov_len = msg_len;
	error = sendmsg(sk1, &outmessage, 0);
	if ((error != -1) || (errno != EMSGSIZE))
       		tst_brkm(TBROK, tst_exit, "Send a message that exceeds "
			 "assoc frag point error:%d errno:%d", error, errno);

	tst_resm(TPASS, "Send a message that exceeds assoc frag point");

	/* Enable Fragmentation. */
	disable_frag = 0;
	test_setsockopt(sk1, SCTP_DISABLE_FRAGMENTS, &disable_frag,
			sizeof(disable_frag));

	msg_cnt = sizeof(msg_sizes) / sizeof(int);

	/* Send and receive the messages of different sizes specified in the
	 * msg_sizes array in a loop.
	 */
	for (i = 0; i < msg_cnt; i++) {

		msg_len = msg_sizes[i];
		msg_buf = test_build_msg(msg_len);
        	outmessage.msg_iov->iov_base = msg_buf;
        	outmessage.msg_iov->iov_len = msg_len;
        	bytes_sent = test_sendmsg(sk1, &outmessage, 0, msg_len);
		
		tst_resm(TINFO, "Sent %d byte message", bytes_sent);

        	inmessage.msg_controllen = sizeof(incmsg);
        	error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
		/* Handle Partial Reads. */ 
		if (inmessage.msg_flags & MSG_EOR) {
	        	test_check_msg_data(&inmessage, error, bytes_sent,
					    MSG_EOR, stream, ppid);
			tst_resm(TINFO, "Received %d byte message", error);
		} else {
			int remain;

	        	test_check_msg_data(&inmessage, error, error, 0,
					    stream, ppid);
			tst_resm(TINFO, "Received %d byte message", error);

			/* Read the remaining message. */
			inmessage.msg_controllen = sizeof(incmsg);
			remain = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	        	test_check_msg_data(&inmessage, remain,
					    bytes_sent - error,
					    MSG_EOR, stream, ppid);
			tst_resm(TINFO, "Received %d byte message", error);
		}

		free(msg_buf);
	}

	tst_resm(TPASS, "Send/Receive fragmented messages");

        /* Shut down the link.  */
        close(sk1);

        /* Get the shutdown complete notification. */
	inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);
				
        close(sk2);

        /* Indicate successful completion.  */
       	return 0; 
}
