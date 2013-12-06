/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2002, 2003
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
 * Written or modified by:
 *    Sridhar Samudrala		<sri@us.ibm.com>
 *
 * Any bugs reported to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release.
 */

/* This is a kernel test to verify
 * 1. MSG_EOR flag is set correctly when a single message is read using multiple
 *    recvmsg() calls. 
 * 2. MSG_PEEK support. 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
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
main(int argc, char *argv[])
{
	int svr_sk, clt_sk;
	struct sockaddr_in svr_loop, clt_loop;
	struct iovec iov, out_iov;
	struct msghdr inmessage, outmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	int error, msglen, i;
	char *big_buffer;
	void *msg_buf;

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered.  
	 */ 
	setvbuf(stdout, NULL, _IONBF, 0); 

	/* Initialize the server and client addresses. */ 
	svr_loop.sin_family = AF_INET;
	svr_loop.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	svr_loop.sin_port = htons(SCTP_TESTPORT_1);
	clt_loop.sin_family = AF_INET;
	clt_loop.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	clt_loop.sin_port = htons(SCTP_TESTPORT_2);

	/* Create and bind the server socket.  */
        svr_sk = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	test_bind(svr_sk, (struct sockaddr *)&svr_loop, sizeof(svr_loop));

	/* Mark server socket as being able to accept new associations.  */
	test_listen(svr_sk, 1);

	/* Create and bind the client sockets.  */
	clt_sk = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	test_bind(clt_sk, (struct sockaddr *)&clt_loop, sizeof(clt_loop));

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(svr_sk);
	test_enable_assoc_change(clt_sk);

	/* Send a message. This will create the association.  */
	memset(&outmessage, 0, sizeof(outmessage));	
	outmessage.msg_name = &svr_loop;
	outmessage.msg_namelen = sizeof(svr_loop);
	outmessage.msg_iov = &out_iov;
	outmessage.msg_iovlen = 1;
	msg_buf = test_build_msg(30000);
	outmessage.msg_iov->iov_base = msg_buf;
	outmessage.msg_iov->iov_len = 30000;
	test_sendmsg(clt_sk, &outmessage, 0, 30000);

	/* Initialize inmessage for all receives. */
	big_buffer = test_malloc(REALLY_BIG);
        memset(&inmessage, 0, sizeof(inmessage));
	iov.iov_base = big_buffer;
	iov.iov_len = 2000;
	inmessage.msg_iov = &iov;
	inmessage.msg_iovlen = 1;
	inmessage.msg_control = incmsg;

	/* Receive COMM_UP on clt_sk. */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(clt_sk, &inmessage, 0);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	

	/* Receive COMM_UP on svr_sk. */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	

	/* Read the 30000 byte message using multiple recvmsg() calls in a
	 * loop with 2000 bytes per read. 
	 */
	for (i = 0, msglen = 30000; i < 15; i++, msglen-=2000) {
		iov.iov_len = REALLY_BIG;
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(svr_sk, &inmessage, MSG_PEEK);
		test_check_msg_data(&inmessage, error, msglen,
				    MSG_EOR, 0, 0);

		iov.iov_len = 2000;
		inmessage.msg_controllen = sizeof(incmsg);
		error = test_recvmsg(svr_sk, &inmessage, MSG_WAITALL);
		test_check_msg_data(&inmessage, error, 2000,
				    ((i==14)?MSG_EOR:0), 0, 0);
	}

	tst_resm(TPASS, "recvmsg with MSG_PEEK flag");
	tst_resm(TPASS, "MSG_EOR in msg_flags set correctly");

	close(svr_sk);
	close(clt_sk);

        /* Indicate successful completion.  */
        return 0;
}
