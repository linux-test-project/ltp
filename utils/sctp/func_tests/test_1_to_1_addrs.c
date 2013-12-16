/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the sctp_getladdrs (), sctp_freealddrs (),
 * sctp_getpaddrs (), sctp_freeapaddrs () for 1-1 style sockets
 *
 * sctp_getladdrs () Tests:
 * -----------------------
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: Socket of different protocol
 * TEST4: Getting the local addresses
 *
 * sctp_freealddrs () Tests:
 * ------------------------
 * TEST5: Freeing the local address
 *
 * sctp_getpaddrs () Tests:
 * -----------------------
 * TEST6: Bad socket descriptor
 * TEST7: Invalid socket
 * TEST8: Socket of different protocol
 * TEST9: Getting the peers addresses
 *
 * sctp_freeapddrs () Tests:
 * ------------------------
 * TEST10: Freeing the peer's address
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
 * be incorporated into the next SCTP release.
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
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 10;
int TST_CNT = 0;

int
main(int argc, char *argv[])
{
        int error;
	socklen_t len;
	int lstn_sk,clnt_sk,acpt_sk,pf_class,sk1;
	struct msghdr outmessage;
        struct msghdr inmessage;
        char *message = "hello, world!\n";
        struct iovec iov_rcv;
        struct sctp_sndrcvinfo *sinfo;
        int msg_count;
        char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
        struct cmsghdr *cmsg;
        struct iovec out_iov;
        char * buffer_rcv;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	struct sockaddr *laddrs, *paddrs;
	int fd, err_no = 0;
	char filename[21];

        struct sockaddr_in conn_addr,lstn_addr,acpt_addr;
	struct sockaddr_in *addr;

	/* Rather than fflush() throughout the code, set stdout to
         * be unbuffered.
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        pf_class = PF_INET;

	/*Creating a regular socket*/
	clnt_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*Creating a listen socket*/
        lstn_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        conn_addr.sin_port = htons(SCTP_TESTPORT_1);

	lstn_addr.sin_family = AF_INET;
        lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	/*Binding the listen socket*/
	test_bind(lstn_sk, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

	/*Listening many sockets as we are calling too many connect here*/
	test_listen(lstn_sk, 1);

	len = sizeof(struct sockaddr_in);
	
	test_connect(clnt_sk, (struct sockaddr *) &conn_addr, len);

	acpt_sk = test_accept(lstn_sk, (struct sockaddr *) &acpt_addr, &len);

	memset(&inmessage, 0, sizeof(inmessage));
        buffer_rcv = malloc(REALLY_BIG);

        iov_rcv.iov_base = buffer_rcv;
        iov_rcv.iov_len = REALLY_BIG;
        inmessage.msg_iov = &iov_rcv;
        inmessage.msg_iovlen = 1;
        inmessage.msg_control = incmsg;
        inmessage.msg_controllen = sizeof(incmsg);

        msg_count = strlen(message) + 1;

	memset(&outmessage, 0, sizeof(outmessage));
        outmessage.msg_name = &lstn_addr;
        outmessage.msg_namelen = sizeof(lstn_addr);
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
        outmessage.msg_iov->iov_len = msg_count;

	test_sendmsg(clnt_sk, &outmessage, MSG_NOSIGNAL, msg_count);

	test_recvmsg(acpt_sk, &inmessage, MSG_NOSIGNAL);

	/*sctp_getladdrs() TEST1: Bad socket descriptor, EBADF Expected error*/
	error = sctp_getladdrs(-1, 0, &laddrs);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "sctp_getladdrs with a bad socket "
			 "descriptor error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "sctp_getladdrs() with a bad socket descriptor - "
		 "EBADF");

	/*sctp_getladdrs() TEST2: Invalid socket, ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = sctp_getladdrs(fd, 0, &laddrs);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "sctp_getladdrs with invalid socket "
			 "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "sctp_getladdrs() with invalid socket - ENOTSOCK");

	/*sctp_getladdrs() TEST3: socket of different protocol
	EOPNOTSUPP Expected error*/
        sk1 = socket(pf_class, SOCK_STREAM, IPPROTO_IP);
	error = sctp_getladdrs(sk1, 0, &laddrs);
	if (error != -1 || errno != EOPNOTSUPP)
		tst_brkm(TBROK, tst_exit, "sctp_getladdrs with socket of "
			 "different protocol error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "sctp_getladdrs() with socket of different protocol - "
		 "EOPNOTSUPP");

	/*sctp_getladdrs() TEST4: Getting the local addresses*/
	error = sctp_getladdrs(lstn_sk, 0, &laddrs);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "sctp_getladdrs with valid socket "
			 "error:%d, errno:%d", error, errno);

	addr = (struct sockaddr_in *)laddrs;
	if (addr->sin_port != lstn_addr.sin_port || 
	    addr->sin_family != lstn_addr.sin_family || 
	    addr->sin_addr.s_addr != lstn_addr.sin_addr.s_addr)
		tst_brkm(TBROK, tst_exit, "sctp_getladdrs comparision failed");

	tst_resm(TPASS, "sctp_getladdrs() - SUCCESS");

	/*sctp_freealddrs() TEST5: freeing the local address*/
	if ((sctp_freeladdrs(laddrs)) < 0)
		tst_brkm(TBROK, tst_exit, "sctp_freeladdrs "
			 "error:%d, errno:%d", error, errno);
		
	tst_resm(TPASS, "sctp_freeladdrs() - SUCCESS");

	/*sctp_getpaddrs() TEST6: Bad socket descriptor, EBADF Expected error*/
	error = sctp_getpaddrs(-1, 0, &paddrs);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "sctp_getpaddrs with a bad socket "
			 "descriptor error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "sctp_getpaddrs() with a bad socket descriptor - "
		 "EBADF");

	/*sctp_getpaddrs() TEST7: Invalid socket, ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = sctp_getpaddrs(fd, 0, &paddrs);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "sctp_getpaddrs with invalid socket "
			 "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "sctp_getpaddrs() with invalid socket - ENOTSOCK");
	
	/*sctp_getpaddrs() TEST8: socket of different protocol
	EOPNOTSUPP Expected error*/
	error = sctp_getpaddrs(sk1, 0, &laddrs);
	if (error != -1 || errno != EOPNOTSUPP)
		tst_brkm(TBROK, tst_exit, "sctp_getpaddrs with socket of "
			 "different protocol error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "sctp_getpaddrs() with socket of different protocol - "
		 "EOPNOTSUPP");
	
	/*sctp_getpaddrs() TEST9: Getting the peer addresses*/
	error = sctp_getpaddrs(acpt_sk, 0, &paddrs);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "sctp_getpaddrs with valid socket "
			 "error:%d, errno:%d", error, errno);
	
	addr = (struct sockaddr_in *)paddrs;
	if (addr->sin_port != acpt_addr.sin_port ||
            addr->sin_family != acpt_addr.sin_family || 
            addr->sin_addr.s_addr != acpt_addr.sin_addr.s_addr)
		tst_brkm(TBROK, tst_exit, "sctp_getpaddrs comparision failed");

	tst_resm(TPASS, "sctp_getpaddrs() - SUCCESS");

	/*sctp_freeapddrs() TEST10: freeing the peer address*/
	if ((sctp_freepaddrs(paddrs)) < 0)
		tst_brkm(TBROK, tst_exit, "sctp_freepaddrs "
			 "error:%d, errno:%d", error, errno);
		
	tst_resm(TPASS, "sctp_freepaddrs() - SUCCESS");

	close(clnt_sk);

	return 0;
}
