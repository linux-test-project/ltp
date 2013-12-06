/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the sendto () call 
 * for 1-1 style sockets
 *
 * TEST1: Sending data from client socket to server socket
 * TEST2: Sending data from accept (server) socket to client socket
 * TEST3: Sending data from unconnected client socket to server
 * TEST4: sending partial data from a buffer
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
int TST_TOTAL = 4;
int TST_CNT = 0;

int
main(int argc, char *argv[])
{
        int msg_count;
	socklen_t len;
	int sk,sk1,pf_class,lstn_sk,acpt_sk,flag;
        char *message = "hello, world!\n";
        char *message_rcv;
        int count;
	
        struct sockaddr_in conn_addr,lstn_addr,svr_addr;

	/* Rather than fflush() throughout the code, set stdout to
         * be unbufferd
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        pf_class = PF_INET;

        sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

        lstn_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	message_rcv = malloc(512);
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

	msg_count = strlen(message) + 1;

	/*sendto() TEST1: Sending data from client socket to server socket*/
	count = sendto(sk, message, msg_count, flag,
		       (const struct sockaddr *) &conn_addr, len);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "sendto from client to server "
                         "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendto() from client to server - SUCCESS");

	test_recv(acpt_sk, message_rcv, msg_count, flag);

	strncpy(message_rcv,"\0",512);

	/*sendto() TEST2: Sending data from accept socket to client socket*/
	count = sendto(acpt_sk, message, msg_count, flag,
		       (const struct sockaddr *) &svr_addr, len);
	if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "sendto from accept socket to client "
                         "count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendto() from accept socket to client - SUCCESS");

	test_recv(sk, message_rcv, msg_count, flag);

        close(sk);
        close(acpt_sk);

        sk1 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*sendto() TEST3: Sending data from unconnected client socket to
        server socket*/
        count = sendto(sk1, message, msg_count, flag,
		       (const struct sockaddr *) &conn_addr, len);
        if (count != msg_count)
		tst_brkm(TBROK, tst_exit, "sendto from unconnected client to "
			 "server count:%d, errno:%d", count, errno);

	tst_resm(TPASS, "sendto() from unconnected client to server - SUCCESS");

        acpt_sk = test_accept(lstn_sk, (struct sockaddr *)&svr_addr, &len);

        test_recv(acpt_sk, message_rcv, msg_count, flag);

	/*send() TEST4: Sending less number of data from the buffer*/
	/*Sending only 5 bytes so that only hello is received*/
	test_sendto(sk, message, 5 , flag, (const struct sockaddr *)&conn_addr,
		    len);
	test_recv(acpt_sk, message_rcv, 5, flag);
	
	tst_resm(TPASS, "sendto() partial data from a buffer - SUCCESS");

	close(sk1);
	close(lstn_sk);
	close(acpt_sk);
	return 0;
	
}
