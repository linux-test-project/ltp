/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the Non-Blocking mode of connect(),
 * accept() and recvmsg() calls.
 *
 * TEST1: Non blocking accept return EAGAIN if connect is not called 
 * TEST2: Non blocking connect should return EINPROGRESS
 * TEST3: accept() passes when connect called in Non-blocking mode
 * TEST4: Non blocking recvmsg should return EAGAIN
 * TEST5: recvmsg() should succeed if data present to receive in non blocking
 *	  mode
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
 * Any bugs reported given to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>         /* for sockaddr_in */
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sys/uio.h>
#include <linux/socket.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 5;
int TST_CNT = 0;

int
main(int argc, char *argv[])
{
        int error,msg_count;
	socklen_t len;
	int sk,pf_class,lstn_sk,acpt_sk,flag,cflag,sflag;
	struct msghdr outmessage;
	struct msghdr inmessage;
        char *message = "hello, world!\n";
        struct iovec iov_rcv;
	struct sctp_sndrcvinfo *sinfo;
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg;
        struct iovec out_iov;
	char * buffer_rcv;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	
        struct sockaddr_in conn_addr,lstn_addr,svr_addr;

	/* Rather than fflush() throughout the code, set stdout to
         * be unbufferd
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        pf_class = PF_INET;

        sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

        lstn_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        conn_addr.sin_port = htons(SCTP_TESTPORT_1);

	lstn_addr.sin_family = AF_INET;
        lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	/*Binding the listen socket*/
        test_bind(lstn_sk, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

        /*Listening the socket*/
        test_listen(lstn_sk, 10);

	len = sizeof(struct sockaddr_in);
	flag = MSG_NOSIGNAL;
	
	/*Setting server socket non-blocking*/
	sflag = fcntl(lstn_sk, F_GETFL, 0);
	if (sflag < 0)
		tst_brkm(TBROK, tst_exit, "fcnt F_GETFL failed "
                         "sflag:%d, errno:%d", sflag, errno);

	error = fcntl(lstn_sk, F_SETFL, sflag | O_NONBLOCK);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "fcnt F_SETFL failed "
                         "error:%d, errno:%d", error, errno);

	/* TEST1: accept should return EAGAIN instead blocking. */
	error = accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);
	if (error != -1 || errno != EAGAIN)
		tst_brkm(TBROK, tst_exit, "non-blocking accept "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "non-blocking accept() - EAGAIN");

	/* TEST2: Non Block connect should return EINPROGRESS */
	/*Set client socket as non-blocking*/
	cflag = fcntl(sk, F_GETFL, 0);
	if (cflag < 0)
		tst_brkm(TBROK, tst_exit, "fcnt F_GETFL failed "
                         "cflag:%d, errno:%d", cflag, errno);

	error = fcntl(sk, F_SETFL, sflag | O_NONBLOCK);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "fcnt F_SETFL failed "
                         "error:%d, errno:%d", error, errno);

	error = connect(sk, (const struct sockaddr *) &conn_addr, len);
	if (error != -1 || errno != EINPROGRESS)
		tst_brkm(TBROK, tst_exit, "non-blocking connect "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "non-blocking connect() - EINPROGRESS");

	/* TEST3: Now that connect() called, accept will succeed */
	acpt_sk = accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);
	if (acpt_sk < 0)
		tst_brkm(TBROK, tst_exit, "accept after a non-blocking connect "
                         "error:%d, errno:%d", error, errno);
	
	tst_resm(TPASS, "accept() after a non-blocking connect - SUCCESS");

	memset(&outmessage, 0, sizeof(outmessage));
        outmessage.msg_name = &svr_addr;
        outmessage.msg_namelen = sizeof(svr_addr);
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

        outmessage.msg_iov->iov_base = message;
        outmessage.msg_iov->iov_len = strlen(message) + 1;

	memset(&inmessage, 0, sizeof(inmessage));
        buffer_rcv = malloc(REALLY_BIG);

        iov_rcv.iov_base = buffer_rcv;
        iov_rcv.iov_len = REALLY_BIG;
        inmessage.msg_iov = &iov_rcv;
        inmessage.msg_iovlen = 1;
        inmessage.msg_control = incmsg;
        inmessage.msg_controllen = sizeof(incmsg);

	msg_count = strlen(message) + 1;

	/* TEST4: recvmsg() should return EAGAIN instead blocking */
	error = recvmsg(sk, &inmessage, MSG_WAITALL);
	if ( error != -1 || errno != EAGAIN)
		tst_brkm(TBROK, tst_exit, "non-blocking recvmsg "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "non-blocking recvmsg() - EAGAIN");

	test_sendmsg(acpt_sk, &outmessage, flag, msg_count);

	/* TEST5: recvmsg() should succeed now as data is available. */
	error = test_recvmsg(sk, &inmessage, flag);
        test_check_msg_data(&inmessage, error, msg_count, MSG_EOR, 0, 0);

	tst_resm(TPASS, "non-blocking recvmsg() when data is available - "
		 "SUCCESS");

	close(lstn_sk);
	close(acpt_sk);
	return 0;
}
