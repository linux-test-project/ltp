/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the shutdown() call for 1-1 style sockets
 *
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: shutdown with SHUT_WR flag to disable new send
 * TEST4: shutdown with SHUT_RD flag to disable new receive
 * TEST5: shutdown with SHUT_RDWR flag to disable new receive/send
 * TEST6: Unconnected socket
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
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/socket.h>
#include <netinet/sctp.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 6;
int TST_CNT = 0;

#define MAX_CLIENTS 10

int
main(int argc, char *argv[])
{
	int clnt_sk[MAX_CLIENTS], acpt_sk[MAX_CLIENTS],sk;
	int lstn_sk;
	struct sockaddr_in lstn_addr, acpt_addr;
	socklen_t addrlen;
	int error, i;
        char *message = "hello, world!\n";
	char msgbuf[100];
	int pf_class;
	int fd, err_no = 0;
	char filename[21];

        /* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered.  
	 */ 
	setvbuf(stdout, NULL, _IONBF, 0); 
	setvbuf(stderr, NULL, _IONBF, 0); 

	/* Initialize the server and client addresses. */ 
	pf_class = PF_INET;

	lstn_addr.sin_family = AF_INET;
	lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

        sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
        lstn_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	test_bind(lstn_sk, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

	test_listen(lstn_sk, MAX_CLIENTS);

	for (i = 0; i < MAX_CLIENTS; i++) {
		clnt_sk[i] = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
		test_connect(clnt_sk[i], (struct sockaddr *)&lstn_addr,
			     sizeof(lstn_addr));
	}

	for (i = 0; i < MAX_CLIENTS; i++) {
		addrlen = sizeof(acpt_addr);
		acpt_sk[i] = test_accept(lstn_sk, (struct sockaddr *)&acpt_addr,
					 &addrlen); 
	}

	/*shutdown() TEST1: Bad socket descriptor, EBADF Expected error*/
	error = shutdown(-1, SHUT_WR);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "shutdown with a bad socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "shutdown() with a bad socket descriptor - EBADF");

	/*shutdown() TEST2: Invalid socket, ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = shutdown(fd, SHUT_WR);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "shutdown with an invalid socket "
			 "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "shutdown() with an invalid socket - ENOTSOCK");

	errno = 0;
	/*Do a send first before doing shutdown*/
	test_send(acpt_sk[0], message, strlen(message), 0);

	/*shutdown() TEST3: shutdown with SHUT_WR flag to disable new send*/
	error = shutdown(clnt_sk[0], SHUT_WR);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "shutdown with SHUT_WR flag "
			 "error:%d, errno:%d", error, errno);

	/* Reading on a socket that has received SHUTDOWN should return 0 
	 * indicating EOF.
	 */
	error = recv(acpt_sk[0], msgbuf, 100, 0);
	if ((error != 0) || (errno != 0))
		tst_brkm(TBROK, tst_exit, "recv on a SHUTDOWN received socket "
			 "error:%d, errno:%d", error, errno);

	/* Read the pending message on clnt_sk[0] that was received before
	 * SHUTDOWN call.
	 */  
	test_recv(clnt_sk[0], msgbuf, 100, 0);

	/* No more messages and the association is SHUTDOWN, should fail. */
	error = recv(clnt_sk[0], msgbuf, 100, 0);
	if ((error != -1) || (errno != ENOTCONN))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_WR socket with no "
			 "messages error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "shutdown() with SHUT_WR flag - SUCCESS");

	errno = 0;

	/*shutdown() TEST4: shutdown with SHUT_RD flag to disable new receive*/
	test_shutdown(clnt_sk[1], SHUT_RD);

	error = recv(clnt_sk[1], msgbuf, 100, 0);
	if ((error != 0) || (errno != 0))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_RD socket "
			 "error:%d, errno:%d", error, errno);

	/* Sending a message on SHUT_RD socket. */
	error = test_send(clnt_sk[1], message, strlen(message), 0);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "send on a SHUT_RD socket "
			 "error:%d, errno:%d", error, errno);

	/* Receive the message sent on SHUT_RD socket. */
	test_recv(acpt_sk[1], msgbuf, 100, 0);

	/* Send a message to the SHUT_RD socket. */
	test_send(acpt_sk[1], message, strlen(message), 0);

	/* We should not receive the message as the socket is SHUT_RD */ 
	error = recv(clnt_sk[1], msgbuf, 100, 0);
	if ((error != 0) || (errno != 0))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_RD socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "shutdown() with SHUT_RD flag - SUCCESS");

	/*shutdown() TEST5: shutdown with SHUT_RDWR flag to disable new 
	receive/send*/
        test_shutdown(clnt_sk[2], SHUT_RDWR);

	error = recv(acpt_sk[2], msgbuf, 100, 0);
	if ((error != 0) || (errno != 0))
		tst_brkm(TBROK, tst_exit, "recv on a SHUTDOWN received socket "
			 "error:%d, errno:%d", error, errno);

	error = recv(clnt_sk[2], msgbuf, 100, 0);
	if ((error != 0) || (errno != 0))
		tst_brkm(TBROK, tst_exit, "recv on a SHUT_RDWR socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "shutdown() with SHUT_RDWR flag - SUCCESS");

	/*shutdown() TEST6: Unconnected socket, ENOTCONN Expected error*/
	error = shutdown(sk, SHUT_RD);
	if ((error != -1) || (errno != ENOTCONN))
		tst_brkm(TBROK, tst_exit, "shutdown on an unconnected socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "shutdown() on an unconnected socket - SUCCESS");

	for (i = 0; i < MAX_CLIENTS; i++)
		close(clnt_sk[i]);
	for (i = 0; i < MAX_CLIENTS; i++)
		close(acpt_sk[i]);


	close(lstn_sk);
	close(sk);

	return 0;
}
