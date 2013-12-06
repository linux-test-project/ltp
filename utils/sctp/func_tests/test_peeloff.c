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

/* This is a Functional test to verify the new SCTP interface sctp_peeloff() 
 * that can be used to branch off an association into a separate socket. 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 6;
int TST_CNT = 0;

#define MAX_CLIENTS 10

int
main(int argc, char *argv[])
{
	int svr_sk, clt_sk[MAX_CLIENTS], peeloff_sk[MAX_CLIENTS];
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
	int pf_class;

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered.  
	 */ 
	setvbuf(stdout, NULL, _IONBF, 0); 

#if TEST_V6
	pf_class = PF_INET6;
        svr_loop.v6.sin6_family = AF_INET6;
        svr_loop.v6.sin6_addr = in6addr_loopback;
        svr_loop.v6.sin6_port = htons(SCTP_TESTPORT_1);
#else
	pf_class = PF_INET;
	svr_loop.v4.sin_family = AF_INET;
	svr_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	svr_loop.v4.sin_port = htons(SCTP_TESTPORT_1);
#endif

	/* Create and bind the server socket.  */
        svr_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
	test_bind(svr_sk, &svr_loop.sa, sizeof(svr_loop));

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(svr_sk);

	/* Mark server socket as being able to accept new associations.  */
	test_listen(svr_sk, 1);

	/* Create and bind all the client sockets.  */
	for (i = 0; i < MAX_CLIENTS; i++) {
		clt_sk[i] = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
#if TEST_V6
        	clt_loop[i].v6.sin6_family = AF_INET6;
        	clt_loop[i].v6.sin6_addr = in6addr_loopback;
        	clt_loop[i].v6.sin6_port = htons(SCTP_TESTPORT_2 + i);
#else
		clt_loop[i].v4.sin_family = AF_INET;
		clt_loop[i].v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
		clt_loop[i].v4.sin_port = htons(SCTP_TESTPORT_2 + i);
#endif
		test_bind(clt_sk[i], &clt_loop[i].sa, sizeof(clt_loop[i]));

		test_enable_assoc_change(clt_sk[i]);
	}

        /* Send the first message from all the clients to the server.  This 
	 * will create the associations.  
	 */
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
	outmessage.msg_iov->iov_base = message;
	outmessage.msg_iov->iov_len = strlen(message) + 1;
	for (i = 0; i < MAX_CLIENTS; i++)
		test_sendmsg(clt_sk[i], &outmessage, 0,
					  strlen(message)+1);
       
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
#if 0
		sac = (struct sctp_assoc_change *)iov.iov_base;
		clt_associd[i] = sac->sac_assoc_id;
#endif
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
		sac = (struct sctp_assoc_change *)iov.iov_base;
		svr_associd[i] = sac->sac_assoc_id;

		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(svr_sk, &inmessage, MSG_WAITALL);
		test_check_msg_data(&inmessage, error, strlen(message) + 1,
				    MSG_EOR, stream, ppid);
	}

	/* Branch off all the associations on the server socket to separate
	 * individual sockets.
	 */ 
	for (i = 0; i < MAX_CLIENTS; i++)
		peeloff_sk[i] = test_sctp_peeloff(svr_sk, svr_associd[i]); 

	tst_resm(TPASS, "sctp_peeloff");

	errno = 0;
	/* Verify that a peeled off socket is not allowed to do a listen().  */
	error = listen(peeloff_sk[0], 1);
	if (error != -1)
		tst_brkm(TBROK, tst_exit, "listen on a peeled off socket "
			 "error: %d, errno: %d", error, errno); 

	tst_resm(TPASS, "listen on a peeled off socket");

	errno = 0;
	/* Verify that an association cannot be branched off an already
	 * peeled-off socket.
	 */
	if ((-1 != sctp_peeloff(peeloff_sk[0], svr_associd[0])) ||
	    (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "sctp_peeloff on a peeled off "
			 "socket error:%d, errno:%d",
			 error, errno);

	tst_resm(TPASS, "sctp_peeloff on a peeled off socket");

	/* Send a message from all the client sockets to the server socket. */
	for (i = 0; i < MAX_CLIENTS; i++)
		test_sendmsg(clt_sk[i], &outmessage, 0, strlen(message)+1);

	/* Receive the sent messages on the peeled off server sockets.  */    
	for (i = 0; i < MAX_CLIENTS; i++) {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(peeloff_sk[i], &inmessage, MSG_WAITALL);
		test_check_msg_data(&inmessage, error, strlen(message) + 1,
				    MSG_EOR, stream, ppid);
	}

	tst_resm(TPASS, "Receive msgs on peeled off sockets");

	/* Send a message from all the peeled off server sockets to the client 
	 * sockets. 
	 */
	for (i = 0; i < MAX_CLIENTS; i++) {
		outmessage.msg_name = &clt_loop[i];
		outmessage.msg_namelen = sizeof(clt_loop[i]);
		test_sendmsg(peeloff_sk[i], &outmessage, 0, strlen(message)+1);
	}

	/* Receive the messages sent from the peeled of server sockets on 
	 * the client sockets.
	 */
	for (i = 0; i < MAX_CLIENTS; i++) {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(clt_sk[i], &inmessage, MSG_WAITALL);
		test_check_msg_data(&inmessage, error, strlen(message) + 1,
				    MSG_EOR, stream, ppid);
	}

	tst_resm(TPASS, "Send msgs on peeled off sockets");

	errno = 0;
	/* Verify that a peeled-off socket cannot initialize a new 
	 * association by trying to send a message to a client that is not
	 * associated with the peeled-off socket.
	 * The message is sent to the client that is associated with the
	 * socket.
	 */ 
	outmessage.msg_name = &clt_loop[1];
	outmessage.msg_namelen = sizeof(clt_loop[1]);
	test_sendmsg(peeloff_sk[0], &outmessage, 0, strlen(message)+1);

	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(clt_sk[0], &inmessage, MSG_WAITALL);
	test_check_msg_data(&inmessage, error, strlen(message) + 1,
			    MSG_EOR, stream, ppid);

	tst_resm(TPASS, "peeled off socket cannot initialize a new assoc");

	close(svr_sk);

	/* Close all the peeled off server sockets.  */
	for (i = 0; i < MAX_CLIENTS; i++)
		close(peeloff_sk[i]);

	/* Get the shutdown complete notification from all the client 
	 * sockets.  
	 */
	for (i = 0; i < MAX_CLIENTS; i++) {
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(clt_sk[i], &inmessage, MSG_WAITALL);
		test_check_msg_notification(&inmessage, error,
					    sizeof(struct sctp_assoc_change),
					    SCTP_ASSOC_CHANGE,
					    SCTP_SHUTDOWN_COMP);	

		close(clt_sk[i]);
	}

        /* Indicate successful completion.  */
       	return 0; 
}
