/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2003
 * Copyright (c) 1999-2001 Motorola, Inc.
 *
 * This file is part of the SCTP kernel Implementation
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
 *    Sridhar Samudrala		<sri@us.ibm.com>
 */

/* This is a kernel test to verify the TCP-style socket interfaces. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 22;
int TST_CNT = 0;

#define MAX_CLIENTS 10

int
main(int argc, char *argv[])
{
	int clt_sk[MAX_CLIENTS], accept_sk[MAX_CLIENTS];
	int listen_sk, clt2_sk, accept2_sk;
	sockaddr_storage_t clt_loop[MAX_CLIENTS];
	sockaddr_storage_t svr_loop, accept_loop, clt2_loop;
	socklen_t addrlen;
	int error, i;
        char *message = "hello, world!\n";
	char msgbuf[100];
	int pf_class;
	struct pollfd poll_fd;
	fd_set set;
	struct msghdr outmessage;
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct iovec out_iov;
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sinfo;
	struct msghdr inmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char *big_buffer;
	struct iovec iov;

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered.  
	 */ 
	setvbuf(stdout, NULL, _IONBF, 0); 

	/* Initialize the server and client addresses. */ 
#if TEST_V6
	pf_class = PF_INET6;
        svr_loop.v6.sin6_family = AF_INET6;
        svr_loop.v6.sin6_addr = in6addr_loopback;
        svr_loop.v6.sin6_port = htons(SCTP_TESTPORT_1);
	for (i = 0; i < MAX_CLIENTS; i++) {
        	clt_loop[i].v6.sin6_family = AF_INET6;
        	clt_loop[i].v6.sin6_addr = in6addr_loopback;
        	clt_loop[i].v6.sin6_port = htons(SCTP_TESTPORT_2 + i);
	}
        clt2_loop.v6.sin6_family = AF_INET6;
        clt2_loop.v6.sin6_addr = in6addr_loopback;
        clt2_loop.v6.sin6_port = htons(SCTP_TESTPORT_2 + i);
#else
	pf_class = PF_INET;
	svr_loop.v4.sin_family = AF_INET;
	svr_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	svr_loop.v4.sin_port = htons(SCTP_TESTPORT_1);
	for (i = 0; i < MAX_CLIENTS; i++) {
		clt_loop[i].v4.sin_family = AF_INET;
		clt_loop[i].v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
		clt_loop[i].v4.sin_port = htons(SCTP_TESTPORT_2 + i);
	}
	clt2_loop.v4.sin_family = AF_INET;
	clt2_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	clt2_loop.v4.sin_port = htons(SCTP_TESTPORT_2 + i);
#endif

	/* Create and bind the listening server socket.  */
        listen_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
	test_bind(listen_sk, &svr_loop.sa, sizeof(svr_loop));

	/* Mark listen_sk as being able to accept new associations.  */
	test_listen(listen_sk, MAX_CLIENTS-1);

	/* Create and bind the client sockets.  */
	for (i = 0; i < MAX_CLIENTS; i++) {
		clt_sk[i] = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
		test_bind(clt_sk[i], &clt_loop[i].sa, sizeof(clt_loop[i]));
	}
	clt2_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
	test_bind(clt2_sk, &clt2_loop.sa, sizeof(clt2_loop));

	addrlen = sizeof(accept_loop);
	/* Try to do accept on a non-listening socket. It should fail. */
	error = accept(clt_sk[0], &accept_loop.sa, &addrlen);
	if ((-1 != error) && (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "accept on non-listening socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "accept on non-listening socket");

	/* Try to do a connect from a listening socket. It should fail. */
	error = connect(listen_sk, (struct sockaddr *)&clt_loop[0],
			sizeof(clt_loop[0]));
	if ((-1 != error) && (EISCONN != errno))
		tst_brkm(TBROK, tst_exit, "connect to non-listening socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect to non-listening socket");

	/* Do a blocking connect from clt_sk's to listen_sk */      
	for (i = 0; i < MAX_CLIENTS; i++)
		test_connect(clt_sk[i], &svr_loop.sa, sizeof(svr_loop));

	tst_resm(TPASS, "connect to listening socket");

	/* Verify that no more connect's can be done after the acceptq
	 * backlog has reached the max value.
	 */
	error = connect(clt2_sk, &svr_loop.sa, sizeof(svr_loop));
	if ((-1 != error) && (ECONNREFUSED != errno))
		tst_brkm(TBROK, tst_exit, "connect after max backlog "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect after max backlog");

	/* Extract the associations on the listening socket as new sockets. */
	for (i = 0; i < MAX_CLIENTS; i++) {
		poll_fd.fd = listen_sk;
		poll_fd.events = POLLIN;
		poll_fd.revents = 0;
		error = poll(&poll_fd, 1, -1);
		if ((1 != error) && (1 != poll_fd.revents))
			tst_brkm(TBROK, tst_exit, "Unexpected return value "
				 "with poll, error:%d errno:%d, revents:%d",
				 error, errno, poll_fd.revents);

		addrlen = sizeof(accept_loop);
		accept_sk[i] = test_accept(listen_sk, &accept_loop.sa,
					   &addrlen); 
	}

	tst_resm(TPASS, "accept from listening socket");

	/* Try to do a connect on an established socket. It should fail. */
	error = connect(accept_sk[0], &clt_loop[0].sa, sizeof(clt_loop[0]));
	if ((-1 != error) || (EISCONN != errno))
		tst_brkm(TBROK, tst_exit, "connect on an established socket "
			 "error:%d errno:%d", error, errno);

	tst_resm(TPASS, "connect on an established socket");

	/* Try to do accept on an established socket. It should fail. */
	error = accept(accept_sk[0], &accept_loop.sa, &addrlen);
	if ((-1 != error) && (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "accept on an established socket "
			 "error:%d errno:%d", error, errno);

	error = accept(clt_sk[0], &accept_loop.sa, &addrlen);
	if ((-1 != error) && (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "accept on an established socket "
			 "failure: error:%d errno:%d", error, errno);

	tst_resm(TPASS, "accept on an established socket");

	/* Send and receive a message from the client sockets to the accepted
	 * sockets.
	 */
	for (i = 0; i < MAX_CLIENTS; i++) {
		test_send(clt_sk[i], message, strlen(message), 0);
		test_recv(accept_sk[i], msgbuf, 100, 0);
	}

	tst_resm(TPASS, "client sockets -> accepted sockets");

	/* Send and receive a message from the accepted sockets to the client
	 * sockets.
	 */
	for (i = 0; i < MAX_CLIENTS; i++) {
		test_send(accept_sk[i], message, strlen(message), 0);
		test_recv(clt_sk[i], msgbuf, 100, 0);
	}

	tst_resm(TPASS, "accepted sockets -> client sockets");

	/* Sending a message on a listening socket should fail. */
	error = send(listen_sk, message, strlen(message), MSG_NOSIGNAL);
	if ((-1 != error) || (EPIPE != errno))
		tst_brkm(TBROK, tst_exit, "send on a listening socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "send on a listening socket");

	/* Trying to receive a message on a listening socket should fail. */
	error = recv(listen_sk, msgbuf, 100, 0);
	if ((-1 != error) || (ENOTCONN != errno))
		tst_brkm(TBROK, tst_exit, "recv on a listening socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "recv on a listening socket");

	/* TESTCASES for shutdown() */
	errno = 0;
	test_send(accept_sk[0], message, strlen(message), 0);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(clt_sk[0]);

	/* Do a SHUT_WR on clt_sk[0] to disable any new sends. */
	test_shutdown(clt_sk[0], SHUT_WR);

	/* Reading on a socket that has received SHUTDOWN should return 0 
	 * indicating EOF.
	 */
	error = recv(accept_sk[0], msgbuf, 100, 0);
	if ((0 != error) || (0 != errno))
		tst_brkm(TBROK, tst_exit, "recv on a SHUTDOWN received socket "
			 "error:%d errno:%d", error, errno);

	tst_resm(TPASS, "recv on a SHUTDOWN received socket");

	/* Read the pending message on clt_sk[0] that was received before
	 * SHUTDOWN call.
	 */  
	test_recv(clt_sk[0], msgbuf, 100, 0);

	/* Initialize inmessage for all receives. */
	big_buffer = test_malloc(REALLY_BIG);
	memset(&inmessage, 0, sizeof(inmessage));	
	iov.iov_base = big_buffer;
	iov.iov_len = REALLY_BIG;
	inmessage.msg_iov = &iov;
	inmessage.msg_iovlen = 1;
	inmessage.msg_control = incmsg;
	inmessage.msg_controllen = sizeof(incmsg);

	/* Receive the SHUTDOWN_COMP notification as they are enabled. */
	error = test_recvmsg(clt_sk[0], &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);

	tst_resm(TPASS, "recv SHUTDOWN_COMP notification on a SHUT_WR socket");

	/* No more messages and the association is SHUTDOWN, should fail. */
	error = recv(clt_sk[0], msgbuf, 100, 0);
	if ((-1 != error) || (ENOTCONN != errno))
		tst_brkm(TBROK, tst_exit, "recv on a SHUTDOWN sent socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "recv on a SHUTDOWN sent socket");

	errno = 0;

	/* Do a SHUT_RD on clt_sk[1] to disable any new receives. */
	test_shutdown(clt_sk[1], SHUT_RD);

	error = recv(clt_sk[1], msgbuf, 100, 0);
	if ((0 != error) || (0 != errno))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_RD socket "
			 "error:%d, errno:%d", error, errno);

	/* Sending a message on SHUT_RD socket. */
	test_send(clt_sk[1], message, strlen(message), 0);

	/* Receive the message sent on SHUT_RD socket. */
	test_recv(accept_sk[1], msgbuf, 100, 0);

	/* Send a message to the SHUT_RD socket. */
	test_send(accept_sk[1], message, strlen(message), 0);

	/* We should not receive the message as the socket is SHUT_RD */ 
	error = recv(clt_sk[1], msgbuf, 100, 0);
	if ((0 != error) || (0 != errno))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_RD socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "recv on a SHUT_RD socket");

	/* Do a SHUT_RDWR on clt_sk[2] to disable any new sends/receives. */
	test_shutdown(clt_sk[2], SHUT_RDWR);

	error = recv(accept_sk[2], msgbuf, 100, 0);
	if ((0 != error) || (0 != errno))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_RDWR socket "
			 "error:%d, errno:%d", error, errno);

	error = recv(clt_sk[2], msgbuf, 100, 0);
	if ((0 != error) || (0 != errno))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_RDWR socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "recv on a SHUT_RDWR socket");

	error = 0;

	for (i = 0; i < MAX_CLIENTS; i++)
		close(clt_sk[i]);
	for (i = 0; i < MAX_CLIENTS; i++)
		close(accept_sk[i]);

	/* Test case to verify accept of a CLOSED association. */
	/* Do a connect, send and a close to ESTABLISH and CLOSE an
	 * association on the listening socket.
	 */
	test_connect(clt2_sk, &svr_loop.sa, sizeof(svr_loop));

	test_send(clt2_sk, message, strlen(message), 0);

	close(clt2_sk);

	FD_ZERO(&set);
	FD_SET(listen_sk, &set);

	error = select(listen_sk + 1, &set, NULL, NULL, NULL);
	if (1 != error)
		tst_brkm(TBROK, tst_exit, "select error:%d, "
			 "errno: %d", error, errno);

	/* Now accept the CLOSED association waiting on the listening 
	 * socket.
	 */  
	accept2_sk = test_accept(listen_sk, &accept_loop.sa, &addrlen); 

	/* Receive the message sent before doing a close. */
	test_recv(accept2_sk, msgbuf, 100, 0);

	/* Receive EOF indication as there are no more messages and the
	 * socket is SHUTDOWN.
	 */
	error = recv(accept2_sk, msgbuf, 100, 0);
	if ((0 != error) || (0 != errno))
		tst_brkm(TBROK, tst_exit, "Unexpected error return on "
			 "recv(error:%d, errno:%d)", error, errno);

	tst_resm(TPASS, "accept of a CLOSED association");

	/* Trying to send a message over the CLOSED association should
	 * generate EPIPE.
	 */
	error = send(accept2_sk, message, strlen(message), MSG_NOSIGNAL);
	if ((-1 != error) || (EPIPE != errno))
		tst_brkm(TBROK, tst_exit, "send to a CLOSED association "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "send to a CLOSED association");

	error = 0;
	close(accept2_sk);

	/* Verify that auto-connect can be done on a TCP-style socket using
	 * sendto/sendmsg.
	 */
	clt2_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
	test_bind(clt2_sk, &clt2_loop.sa, sizeof(clt2_loop));

	/* Do a sendto() without a connect() */
	test_sendto(clt2_sk, message, strlen(message), 0, &svr_loop.sa,
		    sizeof(svr_loop));

	accept2_sk = test_accept(listen_sk, &accept_loop.sa, &addrlen); 

	test_recv(accept2_sk, msgbuf, 100, 0);

	tst_resm(TPASS, "auto-connect using sendto");

	outmessage.msg_name = &svr_loop;
	outmessage.msg_namelen = sizeof(svr_loop);
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

	/* Verify that SCTP_EOF cannot be used to shutdown an association
	 * on a TCP-style socket.
	 */
	sinfo->sinfo_flags |= SCTP_EOF;
	error = sendmsg(clt2_sk, &outmessage, 0);
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "sendmsg with SCTP_EOF flag "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "sendmsg with SCTP_EOF flag");

	/* Verify that SCTP_ABORT cannot be used to abort an association
	 * on a TCP-style socket.
	 */
	sinfo->sinfo_flags |= SCTP_ABORT;
	error = sendmsg(clt2_sk, &outmessage, 0);
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "sendmsg with SCTP_ABORT flag "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "sendmsg with SCTP_ABORT flag");

	/* Verify that a normal message can be sent using sendmsg. */
	outmessage.msg_iov = &out_iov;
	outmessage.msg_iovlen = 1;
	out_iov.iov_base = message;
	out_iov.iov_len = strlen(message) + 1;
	sinfo->sinfo_flags = 0;
	test_sendmsg(clt2_sk, &outmessage, 0, strlen(message)+1);

	test_recv(accept2_sk, msgbuf, 100, 0);
	
	tst_resm(TPASS, "sendmsg with no flags");

	close(clt2_sk);
	close(accept2_sk);
	close(listen_sk);

        /* Indicate successful completion.  */
	return 0;
}
