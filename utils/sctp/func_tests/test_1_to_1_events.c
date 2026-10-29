/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This test tests the events for 1-1 style sockets.
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
 *
 * Any bugs reported given to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>         /* needed by linux/sctp.h */
#include <sys/uio.h>
#include <netinet/in.h>         /* for sockaddr_in */
#include <errno.h>
#include <netinet/sctp.h>
#include <sctputil.h>
#include <string.h>
#include "tst_kernel.h"

char *TCID = __FILE__;
int TST_TOTAL = 4;
int TST_CNT = 0;

int
main(void)
{
	int svr_sk, clt_sk,acpt_sk;
	struct sockaddr_in svr_loop, clt_loop,acpt_loop;
	struct iovec iov, out_iov;
	struct msghdr inmessage, outmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	int error;
	socklen_t len;
	char *big_buffer;
	struct sctp_event_subscribe event;
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sinfo;
	char *message = "hello, world!\n";
	uint32_t ppid;
	uint32_t stream;

	if (tst_check_driver("sctp"))
		tst_brkm(TCONF, tst_exit, "sctp driver not available");

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
	clt_loop.sin_port = htons(SCTP_TESTPORT_1);

	/* Create and bind the server socket.  */
        svr_sk = test_socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	test_bind(svr_sk, (struct sockaddr *) &svr_loop, sizeof(svr_loop));

	/* Mark server socket as being able to accept new associations.  */
	test_listen(svr_sk, 3);

	/* Create the client socket.  */
	clt_sk = test_socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	memset(&event, 0, sizeof(struct sctp_event_subscribe));
	event.sctp_data_io_event = 1;
	event.sctp_association_event = 1;
	event.sctp_shutdown_event = 1;
	len = sizeof(struct sctp_event_subscribe);
	test_setsockopt(svr_sk, SCTP_EVENTS, &event, len);
	test_setsockopt(clt_sk, SCTP_EVENTS, &event, len);

	len = sizeof(struct sockaddr_in);
	test_connect(clt_sk, (struct sockaddr *) &clt_loop, len);
	
	acpt_sk = test_accept(svr_sk, (struct sockaddr *) &acpt_loop, &len);

	/* Build up a msghdr structure we can use for all sending.  */
	memset(&outmessage, 0, sizeof(outmessage));	
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
	outmessage.msg_iov->iov_len = (strlen(message) + 1);

	/* Send . This will create the association*/
	test_sendmsg(clt_sk, &outmessage, 0, strlen(message)+1);

        memset(&inmessage, 0, sizeof(inmessage));
	/* NOW initialize inmessage with enough space for DATA... */
	big_buffer = malloc(REALLY_BIG);
	if (!big_buffer) { DUMP_CORE; }

	/* Let's do a test to do a recvmsg when we are not listening and
	 * when we have no associations.
	 */
	iov.iov_base = big_buffer;
	iov.iov_len = REALLY_BIG;
	inmessage.msg_iov = &iov;
	inmessage.msg_iovlen = 1;
	inmessage.msg_control = incmsg;
	inmessage.msg_controllen = sizeof(incmsg);

	error = test_recvmsg(clt_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage,
                                    error,
                                    sizeof(struct sctp_assoc_change),
                                    SCTP_ASSOC_CHANGE,
                                    SCTP_COMM_UP);
	
	tst_resm(TPASS, "COMM_UP notification on client socket - SUCCESS");

	error = test_recvmsg(acpt_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage,
                                    error,
                                    sizeof(struct sctp_assoc_change),
                                    SCTP_ASSOC_CHANGE,
                                    SCTP_COMM_UP);
	
	tst_resm(TPASS, "COMM_UP notification on server socket - SUCCESS");

	inmessage.msg_control = incmsg;
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(acpt_sk, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(message) + 1,
                            MSG_EOR, stream, ppid);

	tst_resm(TPASS, "Data message on server socket - SUCCESS");

	close(clt_sk);
	error = test_recvmsg(acpt_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage,
                                    error,
                                    sizeof(struct sctp_shutdown_event),
                                    SCTP_SHUTDOWN_EVENT,
                                    0);

	tst_resm(TPASS, "SHUTDOWN notification on accepted socket - SUCCESS");
	close(svr_sk);
	close(acpt_sk);

	return 0;
}
