/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the recvfrom () call for 1-1 style sockets
 *
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: Invalid message pointer
 * TEST4: On a listening socket
 * TEST5: Reading on a socket that received SHUTDOWN
 * TEST6: Reading the pending message on socket that received SHUTDOWN
 * TEST7: No more message and association is shutdown
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
int TST_TOTAL = 7;
int TST_CNT = 0;

int
main(int argc, char *argv[])
{
        int msg_count;
	socklen_t len;
	int sk,pf_class,lstn_sk,acpt_sk, flag;
        char *message = "hello, world!\n";
	char *message_rcv;
        int count;
	int fd, err_no = 0;
	char filename[21];

        struct sockaddr_in conn_addr,lstn_addr,svr_addr;

	/* Rather than fflush() throughout the code, set stdout to
         * be unbuffered.
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

	message_rcv = malloc(512);

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
	
	msg_count = (strlen(message) + 1);

	flag = MSG_NOSIGNAL;
	/*Sending the message*/
	count = test_send(sk, message, msg_count, flag);

	/*recvfrom () TEST1: Bad socket descriptor, EBADF Expected error*/
	count = recvfrom(-1, message_rcv, msg_count, flag,
			 (struct sockaddr *)&svr_addr, &len);
	if (count != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "recvfrom with a bad socket "
			 "descriptor count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvfrom() with a bad socket descriptor - EBADF");

	/*recvfrom () TEST2: Invalid socket , ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	count = recvfrom(fd, message_rcv, msg_count, flag,
			 (struct sockaddr *)&svr_addr, &len);
	if (count == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (count != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "recvfrom with invalid socket "
			 "count:%d, errno:%d", count, err_no);

	tst_resm(TPASS, "recvfrom() with invalid socket - ENOTSOCK");

	/*recvfrom () TEST3: Invalid message pointer EFAULT, Expected error*/
	count = recvfrom(acpt_sk, (char *)-1, msg_count, flag,
			 (struct sockaddr *)&svr_addr, &len);
	if (count != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "recvfrom with invalid message "
			 "pointer count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvfrom() with invalid message ptr - EFAULT");

	/*TEST4: recvfrom on listening socket,ENOTCONN Expected error*/
	count = recvfrom(lstn_sk, message_rcv, msg_count, flag,
			 (struct sockaddr *)&svr_addr, &len);
	if (count != -1 || errno != ENOTCONN)
		tst_brkm(TBROK, tst_exit, "recvfrom on listening socket "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvfrom() on listening socket - ENOTCONN");

	count = test_send(acpt_sk, message, msg_count, flag);

	test_shutdown(sk, SHUT_WR);

	/*recvfrom () TEST5:reading on a socket that received SHUTDOWN*/
	count = recvfrom(acpt_sk, message_rcv, msg_count, flag,
			 (struct sockaddr *)&svr_addr, &len);
	if (count < 0)
		tst_brkm(TBROK, tst_exit, "recvfrom on a socket that has "
			 "received shutdown count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "recvfrom() on a socket that has received shutdown - "
		 "EOF");

	/*recvfrom () TEST6:reading the pending message on socket that sent 
	SHUTDOWN*/
	count = recvfrom(sk, message_rcv, msg_count, flag,
			 (struct sockaddr *)&svr_addr, &len);
	if (count < 0)
		tst_brkm(TBROK, tst_exit, "recvfrom on a socket with pending "
			 "message that has sent shutdown count:%d, errno:%d",
			 count, errno);

	tst_resm(TPASS, "recvfrom() on a socket with pending message that has "
		 "sent shutdown - SUCCESS");

	/*recvfrom () TEST7: No more message and association is shutdown,
	ENOTCONN Expected error*/
	count = recvfrom(sk, message_rcv, msg_count, flag,
			 (struct sockaddr *)&svr_addr, &len);
	if (count != -1 || errno != ENOTCONN)
		tst_brkm(TBROK, tst_exit, "recvfrom on a socket with no "
			 "pending messages and has sent shutdown count:%d, "
			 "errno:%d", count, errno);

	tst_resm(TPASS, "recvfrom() on a socket with no pending messages and "
		 " has sent shutdown - ENOTCONN");

	close(sk);
	close(lstn_sk);
	close(acpt_sk);
	return 0;
	
}
