/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the sendmsg() call for 1-1 style sockets
 *
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: On a listening socket
 * TEST4: Invalid iovec pointer
 * TEST5: Invalid iovec length
 * TEST6: Invalid msghdr pointer
 * TEST7: Invalid sinfo flags
 * TEST8: SCTP_EOF flag set
 * TEST9: SCTP_ABORT flag set
 * TEST10: On a closed association
 *
 * TEST11: Sending data from server socket to client socket
 * TEST12: Sending data from client socket to server socket
 * TEST13: Sending data from unconnected client to server 
 * TEST14: Sending a message on SHUT_RD socket
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
int TST_TOTAL = 14;
int TST_CNT = 0;

int
main(int argc, char *argv[])
{
        socklen_t len;
	int msg_count;
	int sk,sk1,pf_class,lstn_sk,acpt_sk,acpt1_sk, flag;
	struct msghdr outmessage;
        char *message = "hello, world!\n";
	struct sctp_sndrcvinfo *sinfo;
        int count;
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg;
        struct iovec out_iov;
	struct msghdr inmessage;
	char * buffer_rcv;
        struct sockaddr_in conn_addr,lstn_addr,svr_addr;
        struct iovec iov_rcv;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	int fd, err_no = 0;
	char filename[21];

	/* Rather than fflush() throughout the code, set stdout to
         * be unbuffered.
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        pf_class = PF_INET;

        sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

        sk1 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

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
	
	test_connect(sk, (struct sockaddr *) &conn_addr, len);

	acpt_sk = test_accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);

	memset(&outmessage, 0, sizeof(outmessage));
        outmessage.msg_name = &conn_addr;
        outmessage.msg_namelen = sizeof(conn_addr);
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

	flag = MSG_NOSIGNAL;
	/*sendmsg () TEST1: Bad socket descriptor, EBADF Expected error*/
	count = sendmsg(-1, &outmessage, flag);
	if (count != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "sendmsg with a bad socket "
			 "descriptor count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() with a bad socket descriptor - EBADF");
	
	/*sendmsg () TEST2: Invalid socket, ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	count = sendmsg(fd, &outmessage, flag);
	if (count == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (count != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "sendmsg with invalid socket "
			 "count:%d, errno:%d", count, err_no);

	tst_resm(TPASS, "sendmsg() with invalid socket - ENOTSOCK");

	/*sendmsg () TEST3: sendmsg on listening socket, EPIPE Expected error*/
	count = sendmsg(lstn_sk, &outmessage, flag);
	if (count != -1 || errno != EPIPE)
		tst_brkm(TBROK, tst_exit, "sendmsg on a listening socket "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() on a listening socket - EPIPE");

	/*sendmsg () TEST4: Invalid iovec pointer EFAULT, Expected error*/
	outmessage.msg_iov = (struct iovec *)-1;
	count = sendmsg(sk, &outmessage, flag);
	if (count != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "sendmsg with invalid iovec "
			 "pointer count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() with invalid iovec ptr - EFAULT");
	
	outmessage.msg_iov = &out_iov;

	/*sendmsg () TEST5: Invalid iovec count EINVAL, Expected error*/
        outmessage.msg_iovlen = 0;
	count = sendmsg(sk, &outmessage, flag);
	if (count != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "sendmsg with invalid iovec "
			 "length count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() with invalid iovec length - EINVAL");

	outmessage.msg_iovlen = 1;
	
	/*sendmsg () TEST6: Invalid msghdr pointer EFAULT, Expected error*/
	count = sendmsg(sk, (struct msghdr *)-1, flag);
	if (count != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "sendmsg with invalid msghdr "
			 "pointer count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() with invalid msghdr ptr - EFAULT");

	/*sendmsg () TEST7: Invalid sinfo flag EINVAL, Expected error*/
	sinfo->sinfo_flags = 999;
	count = sendmsg(sk, &outmessage, -1);
	if (count != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "sendmsg with invalid sinfo "
			 "flags count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() with invalid sinfo flags - EINVAL");

	/*sendmsg () TEST8: SCTP_EOF flag EINVAL, Expected error*/
	sinfo->sinfo_flags = SCTP_EOF;
	count = sendmsg(sk, &outmessage, flag);
	if (count != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "sendmsg with SCTP_EOF flag "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() with SCTP_EOF flag - EINVAL");

	/*sendmsg () TEST9: SCTP_ABORT flag EINVAL, Expected error*/
	sinfo->sinfo_flags = SCTP_ABORT;
	count = sendmsg(sk, &outmessage, flag);
	if (count != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "sendmsg with SCTP_ABORT flag "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() with SCTP_ABORT flag - EINVAL");

	sinfo->sinfo_flags = 0; 
	
	test_connect(sk1, (struct sockaddr *) &lstn_addr, len);
		 
	test_sendmsg(sk1, &outmessage, flag, strlen(message)+1);

	close(sk1);
	acpt1_sk = test_accept(lstn_sk, (struct sockaddr *)&conn_addr, &len);

	/*sendmsg () TEST10:sendmsg on closed association, EPIPE Expected error*/
	count = sendmsg(acpt1_sk, &outmessage, flag);
	if (count != -1 || errno != EPIPE)
		tst_brkm(TBROK, tst_exit, "sendmsg on a closed association "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() on a closed association - EPIPE");

	close(acpt1_sk);
	close(sk);
	close(lstn_sk);
	close(acpt_sk);

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
	
	test_connect(sk, (struct sockaddr *) &conn_addr, len);

	acpt_sk = test_accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);

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

	/*sendmsg() TEST11: Sending data from server socket to client socket*/
	count = sendmsg(acpt_sk, &outmessage, flag);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "sendmsg from accept socket to "
			 "client count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() from accept socket to client - SUCCESS");

	count = test_recvmsg(sk, &inmessage, flag);
        test_check_msg_data(&inmessage, count, msg_count, MSG_EOR, 0, 0);

        outmessage.msg_name = &conn_addr;
        outmessage.msg_namelen = sizeof(conn_addr);
	/*sendmsg() TEST12: Sending data from client socket to server socket*/
	count = sendmsg(sk, &outmessage, flag);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "sendmsg from client to server "
                         "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() from client to server - SUCCESS");

	count = test_recvmsg(acpt_sk, &inmessage, flag);
        test_check_msg_data(&inmessage, count, msg_count, MSG_EOR, 0, 0);

        outmessage.msg_name = &conn_addr;
        outmessage.msg_namelen = sizeof(conn_addr);
	close(sk);
	close(acpt_sk);
	sk1 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*sendmsg() TEST13: Sending data from unconnected client socket to 
	server socket*/
	count = sendmsg(sk1, &outmessage, flag);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "sendmsg from unconnected client to "
			 "server count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() from unconnected clt to server - SUCCESS");

	acpt_sk = test_accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);

	count = test_recvmsg(acpt_sk, &inmessage, flag);
        test_check_msg_data(&inmessage, count, msg_count, MSG_EOR, 0, 0);

	test_shutdown(sk1, SHUT_RD);

	/*sendmsg() TEST14: Sending a message on SHUT_RD socket*/
	count = sendmsg(sk1, &outmessage, flag);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "sendmsg on a SHUT_RD socket "
                         "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendmsg() on a SHUT_RD socket - SUCCESS");

	count = test_recvmsg(acpt_sk, &inmessage, flag);
        test_check_msg_data(&inmessage, count, msg_count, MSG_EOR, 0, 0);

	close(sk1);
	close(lstn_sk);
	close(acpt_sk);
	return 0;
}
