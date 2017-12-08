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
 *    Jon Grimm		 <jgrimm@us.ibm.com>
 *    Sridhar Samudrala  <sri@us.ibm.com>
 */

/*
 * This is a basic functional test for the SCTP kernel 
 * implementation of sndrcvinfo.sinfo_timetolive.
 *
 * 1) Create two sockets, the listening socket sets its RECVBUF small
 * 2) Create a connection.  Send enough data to the non-reading listener
 * to fill the RCVBUF.
 * 5) Set sinfo_timetolive on a message and send.
 * 6) Disable sinfo_timetolive on a message and send.
 * 7) Wait sinfo_timetolive.
 * 8) Read out all the data at the receiver.
 * 9) Make sure timed out message did not make it.
 * 10) Make sure that the message with no timeout makes it to the receiver.
 *
 * Also test with SEND_FAILED notifications.  Also, use a fragmented
 * message so as to also exercise the SEND_FAILED of fragmentation
 * code.
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
int TST_TOTAL = 6;
int TST_CNT = 0;

/* This is the size of our RCVBUF */
#define SMALL_RCVBUF 3000

/* MAX segment size */
#define SMALL_MAXSEG 500

/* RWND_SLOP is the extra data that fills up the rwnd */
#define RWND_SLOP 100
static char *fillmsg = NULL;
static char *ttlmsg = "This should time out!\n";
static char *nottlmsg = "This should NOT time out!\n";
static char ttlfrag[SMALL_MAXSEG*3] = {0};
static char *message = "Hello world\n";

int main(int argc, char *argv[])
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
        int error;
	int pf_class;
	uint32_t ppid;
	uint32_t stream;
	sctp_assoc_t associd1;
	struct sctp_assoc_change *sac;
	struct sctp_event_subscribe subscribe;
	char *big_buffer;
	int offset;
	struct sctp_send_failed *ssf;
	socklen_t len; /* Really becomes 2xlen when set. */
	int orig_len; 
	struct sctp_status gstatus;

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

	len = sizeof(int);
	error = getsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &orig_len,
			   &len);
	if (error)
		tst_brkm(TBROK, tst_exit, "can't get rcvbuf size: %s",
			strerror(errno));
	/* Set the MAXSEG to something smallish. */
	{
		int val = SMALL_MAXSEG;
		test_setsockopt(sk1, SCTP_MAXSEG, &val, sizeof(val));
	}

	memset(&subscribe, 0, sizeof(subscribe));
	subscribe.sctp_data_io_event = 1;
	subscribe.sctp_association_event = 1;
	subscribe.sctp_send_failure_event = 1;
	test_setsockopt(sk1, SCTP_EVENTS, &subscribe, sizeof(subscribe));
	test_setsockopt(sk2, SCTP_EVENTS, &subscribe, sizeof(subscribe));

        /* Bind these sockets to the test ports.  */
        test_bind(sk1, &loop1.sa, sizeof(loop1));
        test_bind(sk2, &loop2.sa, sizeof(loop2));

	/*
	 * This code sets the associations RWND very small so we can
	 * fill it.  It does this by manipulating the rcvbuf as follows:
	 * 1) Reduce the rcvbuf size on the socket
	 * 2) create an association so that we advertize rcvbuf/2 as
	 *    our initial rwnd
	 * 3) raise the rcvbuf value so that we don't drop data wile 
	 *    receiving later data
	 */
	len = SMALL_RCVBUF;
	error = setsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &len,
			   sizeof(len));
	if (error)
		tst_brkm(TBROK, tst_exit, "setsockopt(SO_RCVBUF): %s",
			 strerror(errno));

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
        outmessage.msg_iov->iov_base = message;
        outmessage.msg_iov->iov_len = strlen(message) + 1;
        test_sendmsg(sk1, &outmessage, 0, strlen(message)+1);

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
	sac = (struct sctp_assoc_change *)iov.iov_base;
	associd1 = sac->sac_assoc_id;

	/* restore the rcvbuffer size for the receiving socket */
	error = setsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &orig_len,
			   sizeof(orig_len));

	if (error)
		tst_brkm(TBROK, tst_exit, "setsockopt(SO_RCVBUF): %s",
			strerror(errno));

        /* Get the first data message which was sent.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(message) + 1,
			    MSG_EOR, stream, ppid);

	/* Figure out how big to make our fillmsg */
	len = sizeof(struct sctp_status);
	memset(&gstatus,0,sizeof(struct sctp_status));
	gstatus.sstat_assoc_id = associd1;
	error = getsockopt(sk1, IPPROTO_SCTP, SCTP_STATUS, &gstatus, &len);
	
	if (error)
		tst_brkm(TBROK, tst_exit, "can't get rwnd size: %s",
			strerror(errno));
	tst_resm(TINFO, "Creating fillmsg of size %d",
		 gstatus.sstat_rwnd+RWND_SLOP);
	fillmsg = malloc(gstatus.sstat_rwnd+RWND_SLOP);	

	/* Send a fillmsg */
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
	memset(fillmsg, 'X', gstatus.sstat_rwnd+RWND_SLOP);
	fillmsg[gstatus.sstat_rwnd+RWND_SLOP-1] = '\0';
	outmessage.msg_iov->iov_base = fillmsg;
	outmessage.msg_iov->iov_len = gstatus.sstat_rwnd+RWND_SLOP;
	outmessage.msg_name = NULL;
	outmessage.msg_namelen = 0;
	sinfo->sinfo_assoc_id = associd1;
	sinfo->sinfo_timetolive = 0;
	test_sendmsg(sk1, &outmessage, MSG_NOSIGNAL,
			 gstatus.sstat_rwnd+RWND_SLOP);

	/* Now send the message with timeout. */
	sinfo->sinfo_ppid = ppid;
	sinfo->sinfo_stream = stream;
	outmessage.msg_iov->iov_base = ttlmsg;
        outmessage.msg_iov->iov_len = strlen(ttlmsg) + 1;
	outmessage.msg_name = NULL;
	outmessage.msg_namelen = 0;
	sinfo->sinfo_assoc_id = associd1;
	sinfo->sinfo_timetolive = 2000;
	test_sendmsg(sk1, &outmessage, MSG_NOSIGNAL, strlen(ttlmsg) + 1);

	tst_resm(TPASS, "Send a message with timeout");

	/* Next send a message with no timeout. */
	sinfo->sinfo_ppid = ppid;
	sinfo->sinfo_stream = stream;
	outmessage.msg_iov->iov_base = nottlmsg;
        outmessage.msg_iov->iov_len = strlen(nottlmsg) + 1;
	outmessage.msg_name = NULL;
	outmessage.msg_namelen = 0;
	sinfo->sinfo_assoc_id = associd1;
	sinfo->sinfo_timetolive = 0;
	test_sendmsg(sk1, &outmessage, MSG_NOSIGNAL, strlen(nottlmsg)+1);

	tst_resm(TPASS, "Send a message with no timeout");

	/* And finally a fragmented message that will time out. */
	sinfo->sinfo_ppid = ppid;
	sinfo->sinfo_stream = stream;
	memset(ttlfrag, '0', sizeof(ttlfrag));
	ttlfrag[sizeof(ttlfrag)-1] = '\0';
	outmessage.msg_iov->iov_base = ttlfrag;
        outmessage.msg_iov->iov_len = sizeof(ttlfrag);
	outmessage.msg_name = NULL;
	outmessage.msg_namelen = 0;
	sinfo->sinfo_assoc_id = associd1;
	sinfo->sinfo_timetolive = 2000;
	test_sendmsg(sk1, &outmessage, MSG_NOSIGNAL, sizeof(ttlfrag));

	tst_resm(TPASS, "Send a fragmented message with timeout");

	/* Sleep waiting for the message to time out. */
	tst_resm(TINFO, " **  SLEEPING for 3 seconds **");
	sleep(3);

	/* Read the fillmsg snuck in between the ttl'd messages. */
	do {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	} while (!(inmessage.msg_flags & MSG_EOR));

	/* Now get the message that did NOT time out. */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_data(&inmessage, error, strlen(nottlmsg) + 1,
			    MSG_EOR, stream, ppid);
	if (0 != strncmp(iov.iov_base, nottlmsg, strlen(nottlmsg)+1))
		tst_brkm(TBROK, tst_exit, "Received Wrong Message !!!");

	tst_resm(TPASS, "Receive message with no timeout");

	/* Get the SEND_FAILED notification for the message that DID
	 * time out.
	 */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(sk1, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_send_failed) +
							strlen(ttlmsg) + 1,
				    SCTP_SEND_FAILED, 0);
	ssf = (struct sctp_send_failed *)iov.iov_base;
	if (0 != strncmp(ttlmsg, (char *)ssf->ssf_data, strlen(ttlmsg) + 1))
		tst_brkm(TBROK, tst_exit, "SEND_FAILED data mismatch");

	tst_resm(TPASS, "Receive SEND_FAILED for message with timeout");

	/* Get the SEND_FAILED notification for the fragmented message that
	 * DID time out.
	 */
	offset = 0;
	do {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(sk1, &inmessage, MSG_WAITALL);
		test_check_msg_notification(&inmessage, error,
					    sizeof(struct sctp_send_failed) +
								  SMALL_MAXSEG,
					    SCTP_SEND_FAILED, 0);
		ssf = (struct sctp_send_failed *)iov.iov_base;
		if (0 != strncmp(&ttlfrag[offset], (char *)ssf->ssf_data,
				 SMALL_MAXSEG))
			tst_brkm(TBROK, tst_exit, "SEND_FAILED data mismatch");
		offset += SMALL_MAXSEG;
	} while (!(ssf->ssf_info.sinfo_flags & 0x01)); /* LAST_FRAG */

	tst_resm(TPASS, "Receive SEND_FAILED for fragmented message with "
		 "timeout");

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
