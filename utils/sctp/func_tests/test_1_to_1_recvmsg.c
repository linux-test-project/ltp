/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the recvmsg() call for 1-1 style sockets
 *
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: Invalid iovec pointer
 * TEST4: Invalid msghdr pointer
 * TEST5: On a listening socket
 * TEST6: Reading on a socket that received SHUTDOWN
 * TEST7: Reading the pending message socket that received SHUTDOWN
 * TEST8: No more message and association is shutdown
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
int TST_TOTAL = 8;
int TST_CNT = 0;

int
main(int argc, char *argv[])
{
        socklen_t len;
	int sk,pf_class,lstn_sk,acpt_sk;
	int flag = 0;
	int fd, err_no = 0;
	char filename[21];
	struct msghdr inmessage;
        char *message = "hello, world!\n";
	struct iovec iov_rcv;
        int count;
	char * buffer_rcv;
        char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char *message1 = "hello, world!\n";

        struct sockaddr_in conn_addr,lstn_addr,svr_addr;

	/* Rather than fflush() throughout the code, set stdout to
         * be unbuffered.
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
	
	test_connect(sk, (struct sockaddr *) &conn_addr, len);

	acpt_sk = test_accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);

	memset(&inmessage, 0, sizeof(inmessage));
        buffer_rcv = malloc(REALLY_BIG);

        iov_rcv.iov_base = buffer_rcv;
        iov_rcv.iov_len = REALLY_BIG;
        inmessage.msg_iov = &iov_rcv;
        inmessage.msg_iovlen = 1;
        inmessage.msg_control = incmsg;
        inmessage.msg_controllen = sizeof(incmsg);

	/*recvmsg () TEST1: Bad socket descriptor, EBADF Expected error*/
	count = recvmsg(-1, &inmessage, flag);
	if (count != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "recvmsg with a bad socket "
			 "descriptor count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvmsg() with a bad socket descriptor - EBADF");

	/*recvmsg () TEST2: Invalid socket , ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	count = recvmsg(fd, &inmessage, flag);
	if (count == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (count != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "recvmsg with invalid socket "
			 "count:%d, errno:%d", count, err_no);

	tst_resm(TPASS, "recvmsg() with invalid socket - ENOTSOCK");

	/*recvmsg () TEST3: Invalid iovec pointer EFAULT, Expected error*/
	inmessage.msg_iov = (struct iovec *)-1;
	count = recvmsg(acpt_sk, &inmessage, flag);
	if (count != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "recvmsg with invalid iovec "
			 "pointer count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvmsg() with invalid iovec ptr - EFAULT");
	
	inmessage.msg_iov = &iov_rcv;

	/*recvmsg () TEST4: Invalid msghdr pointer EFAULT, Expected error*/
	count = recvmsg(acpt_sk, (struct msghdr *)-1, flag);
	if (count != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "recvmsg with invalid msghdr "
			 "pointer count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvmsg() with invalid msghdr ptr - EFAULT");

	/*recvmsg () TEST5:recvmsg on listening socket,ENOTCONN Expected error*/
	count = recvmsg(lstn_sk, &inmessage, flag);
	if (count != -1 || errno != ENOTCONN)
		tst_brkm(TBROK, tst_exit, "recvmsg on listening socket "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvmsg() on listening socket - ENOTCONN");

	count = test_send(acpt_sk, message1, strlen(message), 0);

	test_shutdown(sk, SHUT_WR);

	flag = MSG_NOSIGNAL;
	/*recvmsg () TEST6:reading on a socket that received SHUTDOWN*/
	count = recvmsg(acpt_sk, &inmessage, flag);
	if (count < 0)
		tst_brkm(TBROK, tst_exit, "recvmsg on a socket that has "
			 "received shutdown count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvmsg() on a socket that has received shutdown - "
		 "EOF");

	/*recvmsg () TEST7:reading the pending message socket that sent 
	SHUTDOWN*/
	count = recvmsg(sk, &inmessage, flag);
	if (count < 0)
		tst_brkm(TBROK, tst_exit, "recvmsg on a socket with pending "
			 "message that has sent shutdown count:%d, errno:%d",
			 count, errno);

	tst_resm(TPASS, "recvmsg() on a socket with pending message that has "
		 "sent shutdown - SUCCESS");

	/*recvmsg () TEST8: No more message and association is shutdown,
	ENOTCONN Expected error*/
	count = recvmsg(sk, &inmessage, flag);
	if (count != -1 || errno != ENOTCONN)
		tst_brkm(TBROK, tst_exit, "recvmsg on a socket with no "
			 "pending messages and has sent shutdown count:%d, "
			 "errno:%d", count, errno);

	tst_resm(TPASS, "recvmsg() on a socket with no pending messages and "
		 " has sent shutdown - ENOTCONN");

	close(sk);
	close(lstn_sk);
	close(acpt_sk);
	return 0;
}
