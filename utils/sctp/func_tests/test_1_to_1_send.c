/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the send() call for 1-1 style sockets
 *
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: On a listening socket
 * TEST4: On a closed association
 * TEST5: Invalid message address
 * TEST6: send from client to server 
 * TEST7: send from server to client 
 * TEST8: sending partial data from a buffer
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
        socklen_t len,len_snd;
	int msg_count;
	int sk,sk1,pf_class,lstn_sk,acpt_sk,acpt1_sk, flag, count;
        char *message = "hello, world!\n";
        char *message_rcv;
	int fd, err_no = 0;
	char filename[21];

        struct sockaddr_in conn_addr,lstn_addr,svr_addr;

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

	len_snd = (strlen(message) + 1);

	flag = MSG_NOSIGNAL;
	/*send () TEST1: Bad socket descriptor, EBADF Expected error*/
	count = send(-1, message, len_snd, flag);
	if (count != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "send with a bad socket "
			 "descriptor count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "send() with a bad socket descriptor - EBADF");
	
	/*send () TEST2: Invalid socket, ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	count = send(fd, message, len_snd, flag);
	if (count == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (count != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "send with invalid socket "
			 "count:%d, errno:%d", count, err_no);

	tst_resm(TPASS, "send() with invalid socket - ENOTSOCK");

	/*send () TEST3: send on listening socket, EPIPE Expected error*/
	count = send(lstn_sk, message, len_snd, flag);
	if (count != -1 || errno != EPIPE)
		tst_brkm(TBROK, tst_exit, "send on a listening socket "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "send() on a listening socket - EPIPE");
#if 0
	/*send () TEST4: Invalid message address, EFAULT Expected error*/
       /* FIXME this test should pass. Don't catch why...  */
	count = send(sk, (char *)0x1, len_snd, flag);
	if (count != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "send with invalid message "
			 "pointer count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "send() with invalid message ptr - EFAULT");
#endif

	test_connect(sk1, (struct sockaddr *) &lstn_addr, len);
		 
	count = test_send(sk1, message, len_snd, flag);

	close(sk1);

	acpt1_sk = test_accept(lstn_sk, (struct sockaddr *)&conn_addr, &len);

	/*send () TEST5: send on closed association, EPIPE Expected error*/
	count = send(acpt1_sk, message, len_snd, flag);
	if (count != -1 || errno != EPIPE)
		tst_brkm(TBROK, tst_exit, "send on a closed association "
			 "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "send() on a closed association - EPIPE");

	close(acpt1_sk);
	close(sk);
	close(lstn_sk);
	close(acpt_sk);

        sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

        lstn_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	message_rcv = malloc(512);

	/*Binding the listen socket*/
        test_bind(lstn_sk, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

        /*Listening the socket*/
        test_listen(lstn_sk, 10);

	conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        conn_addr.sin_port = htons(SCTP_TESTPORT_1);

	len = sizeof(struct sockaddr_in);

	test_connect(sk, (struct sockaddr *) &conn_addr, len);

	acpt_sk = test_accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);
	
	msg_count = strlen(message) + 1;

	/*send() TEST6: Sending data from client socket to server socket*/
	count = send(sk, message, msg_count, flag);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "send from client to server "
                         "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "send() from client to server - SUCCESS");

	test_recv(acpt_sk, message_rcv, msg_count, flag);

	strncpy(message_rcv,"\0",512);

	/*send() TEST7: Sending data from accept socket to client socket*/
	count = send(acpt_sk, message, msg_count, flag);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "send from accept socket to client "
                         "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "send() from accept socket to client - SUCCESS");

	test_recv(sk, message_rcv, msg_count, flag);

	/*send() TEST8: Sending less number of data from the buffer*/
	/*Sending only 5 bytes so that only hello is received*/
	test_send(sk, message, 5 , flag);
	test_recv(acpt_sk, message_rcv, 5, flag);
	
	tst_resm(TPASS, "send() partial data from a buffer - SUCCESS");

	/* TEST9: sctp_send with no sinfo */
	test_sctp_send(sk, message, strlen(message) + 1 , NULL, flag);
	test_recv(acpt_sk, message_rcv, strlen(message) + 1, flag);
	tst_resm(TPASS, "sctp_send() with no sinfo - SUCCESS");

	close(sk1);
	close(lstn_sk);
	close(acpt_sk);

	return 0;
}
