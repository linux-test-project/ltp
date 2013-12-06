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
 * Sridhar Samudrala <sri@us.ibm.com>
 */

/* This is a Functional Test to verify autoclose functionality and the
 * socket option SCTP_AUTOCLOSE that can be used to specify the duration in
 * which an idle association is automatically closed. 
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

int
main(int argc, char *argv[])
{
	int sk1, sk2;
	sockaddr_storage_t loop1, loop2;
	struct msghdr inmessage, outmessage;
	struct iovec iov, out_iov;
	int error;
	char *big_buffer;
	char *message = "hello, world!\n";
	uint32_t autoclose;

	/* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered. 
	 */
	setvbuf(stdout, NULL, _IONBF, 0); 

	loop1.v4.sin_family = AF_INET;
	loop1.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	loop1.v4.sin_port = htons(SCTP_TESTPORT_1);

	loop2.v4.sin_family = AF_INET;
	loop2.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	loop2.v4.sin_port = htons(SCTP_TESTPORT_2);

	/* Create the two endpoints which will talk to each other.  */
	sk1 = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	sk2 = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(sk1);
	test_enable_assoc_change(sk2);

	/* Bind these sockets to the test ports.  */
	test_bind(sk1, &loop1.sa, sizeof(loop1));
	test_bind(sk2, &loop2.sa, sizeof(loop2));

	/* Mark sk2 as being able to accept new associations.  */
	test_listen(sk2, 1);

	/* Set the autoclose duration for the associations created on sk1 
	 * and sk2 to be 5 seconds.  
	 */ 
	autoclose = 5;
	test_setsockopt(sk1, SCTP_AUTOCLOSE, &autoclose, sizeof(autoclose));
	test_setsockopt(sk2, SCTP_AUTOCLOSE, &autoclose, sizeof(autoclose));

	/* Send the first message.  This will create the association.  */
	memset(&outmessage, 0, sizeof(outmessage));	
	outmessage.msg_name = &loop2;
	outmessage.msg_namelen = sizeof(loop2);
	outmessage.msg_iov = &out_iov;
	outmessage.msg_iovlen = 1;
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
	inmessage.msg_control = NULL;

	/* Get the communication up message on sk2.  */
	error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	

	/* Get the communication up message on sk1.  */
	error = test_recvmsg(sk1, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	

	/* Get the first message which was sent.  */
	error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_data(&inmessage, error, strlen(message) + 1,
			    MSG_EOR|MSG_CTRUNC, 0, 0);

	tst_resm(TINFO, "Waiting for the associations to close automatically "
		 "in 5 secs");

	/* Get the shutdown complete notification from sk1. */
	error = test_recvmsg(sk1, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);	
				
	/* Get the shutdown complete notification from sk2. */
	error = test_recvmsg(sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);	

	tst_resm(TPASS, "Autoclose of associations");

	/* Shut down the link.  */
	close(sk1);
	close(sk2);

	/* Indicate successful completion.  */
	return 0;
}
