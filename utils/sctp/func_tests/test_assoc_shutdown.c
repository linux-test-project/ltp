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
 *    Sridhar Samudrala <sri@us.ibm.com>
 */

/* This is a functional test to verify the graceful shutdown of an
 * association.
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
int TST_TOTAL = 1;
int TST_CNT = 0;

#define MAX_CLIENTS 10

int
main(int argc, char *argv[])
{
	int svr_sk, clt_sk[MAX_CLIENTS];
	sctp_assoc_t svr_associd[MAX_CLIENTS];
	sockaddr_storage_t svr_loop, clt_loop[MAX_CLIENTS];
	struct iovec iov;
	struct msghdr inmessage;
	struct msghdr outmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sinfo;
        struct iovec out_iov;
        int error;
	uint32_t ppid;
	uint32_t stream;
	struct sctp_assoc_change *sac;
	char *big_buffer;
	int i;
        char *message = "hello, world!\n";
	struct sctp_status status;
	socklen_t status_len;

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered.  
	 */ 
	setvbuf(stdout, NULL, _IONBF, 0); 

	/* Create and bind the server socket.  */
        svr_sk = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);

	svr_loop.v4.sin_family = AF_INET;
	svr_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	svr_loop.v4.sin_port = htons(SCTP_TESTPORT_1);
	test_bind(svr_sk, &svr_loop.sa, sizeof(svr_loop));

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(svr_sk);

	/* Mark server socket as being able to accept new associations.  */
	test_listen(svr_sk, 1);

	/* Create and bind all the client sockets.  */
	for (i = 0; i < MAX_CLIENTS; i++) {
		clt_sk[i] = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);

		clt_loop[i].v4.sin_family = AF_INET;
		clt_loop[i].v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
		clt_loop[i].v4.sin_port = htons(SCTP_TESTPORT_2 + i);
		test_bind(clt_sk[i], &clt_loop[i].sa, sizeof(clt_loop[i]));

		test_enable_assoc_change(clt_sk[i]);
	}

	/* Build up a msghdr structure we can use for all sending.  */
	outmessage.msg_name = &svr_loop;
	outmessage.msg_namelen = sizeof(svr_loop);
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
	out_iov.iov_base = message;
	out_iov.iov_len = strlen(message) + 1;
	
        /* Send the first message from all the clients to the server.  This 
	 * will create the associations.  
	 */
	for (i = 0; i < MAX_CLIENTS; i++)
		test_sendmsg(clt_sk[i], &outmessage, 0, strlen(message)+1);
        
	/* Initialize inmessage for all receives. */
	big_buffer = test_malloc(REALLY_BIG);
        memset(&inmessage, 0, sizeof(inmessage));	
	iov.iov_base = big_buffer;
	iov.iov_len = REALLY_BIG;
	inmessage.msg_iov = &iov;
	inmessage.msg_iovlen = 1;
	inmessage.msg_control = incmsg;

	/* Get the communication up message on all client sockets.  */
	for (i = 0; i < MAX_CLIENTS; i++) {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(clt_sk[i], &inmessage, MSG_WAITALL);
		test_check_msg_notification(&inmessage, error,
					    sizeof(struct sctp_assoc_change),
					    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	}

	/* Get the communication up message and the data message on the
	 * server sockets for all the clients.  
	 */
	for (i = 0; i < MAX_CLIENTS; i++) {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(svr_sk, &inmessage, MSG_WAITALL);
		test_check_msg_notification(&inmessage, error,
					    sizeof(struct sctp_assoc_change),
					    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);		

		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(svr_sk, &inmessage, MSG_WAITALL);
		test_check_msg_data(&inmessage, error, strlen(message)+1,
				    MSG_EOR, stream, ppid);
		sac = (struct sctp_assoc_change *)iov.iov_base;
		svr_associd[i] = sac->sac_assoc_id;
	}

	/* Build up a msghdr structure we can use for all sending.  */
	outmessage.msg_name = NULL;
	outmessage.msg_namelen = 0;
	outmessage.msg_iov = NULL;
	outmessage.msg_iovlen = 0;
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
	sinfo->sinfo_flags |= SCTP_EOF;

	/* Shutdown all the associations of the server socket in a loop.  */
	for (i = 0; i < MAX_CLIENTS; i++) {
		sinfo->sinfo_assoc_id = svr_associd[i];

		/* Verify that the association is present. */
		memset(&status, 0, sizeof(struct sctp_status));
		status.sstat_assoc_id = sinfo->sinfo_assoc_id;
		status_len = sizeof(struct sctp_status);
		error = getsockopt(svr_sk, SOL_SCTP, SCTP_STATUS,
				   &status, &status_len);
		if (error)
			tst_brkm(TBROK, tst_exit,
				 "getsockopt(SCTP_STATUS): %s",
				 strerror(errno));

		/* Call sendmsg() to shutdown the association.  */
		test_sendmsg(svr_sk, &outmessage, 0, 0);

		/* Verify that the association is no longer present.  */
		memset(&status, 0, sizeof(struct sctp_status));
		status.sstat_assoc_id = sinfo->sinfo_assoc_id;
		status_len = sizeof(struct sctp_status);
		error = getsockopt(svr_sk, SOL_SCTP, SCTP_STATUS, 
				   &status, &status_len);
		if ((error != -1) && (errno != EINVAL))
			tst_brkm(TBROK, tst_exit,
				 "getsockopt(SCTP_STATUS) "
				 "error:%d errno:%d", error, errno);
	}

	close(svr_sk);

        /* Get the shutdown complete notification. */
	for (i = 0; i < MAX_CLIENTS; i++) {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(clt_sk[i], &inmessage, MSG_WAITALL);
		test_check_msg_notification(&inmessage, error,
					    sizeof(struct sctp_assoc_change),
					    SCTP_ASSOC_CHANGE,
					    SCTP_SHUTDOWN_COMP);

		close(clt_sk[i]);
	}

	tst_resm(TPASS, "Graceful shutdown of associations using SCTP_EOF"); 

        /* Indicate successful completion.  */
        return 0;
}
