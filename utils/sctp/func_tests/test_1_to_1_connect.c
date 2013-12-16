/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the connect () call for 1-1 style sockets
 *
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: Invalid address
 * TEST4: Invalid address length
 * TEST5: Invalid address family
 * TEST6: Valid blocking connect
 * TEST7: Connect when accept queue is full
 * TEST8: On a listening socket
 * TEST9: On established socket
 * TEST10: Connect to re-establish a closed association. 
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
#include <linux/socket.h>
#include <netinet/in.h>         /* for sockaddr_in */
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <netinet/sctp.h>
#include "sctputil.h"

char *TCID = __FILE__;
int TST_TOTAL = 10;
int TST_CNT = 0;

#define SK_MAX 10

int
main(int argc, char *argv[])
{
        int error,i;
	socklen_t len;
	int sk,lstn_sk,clnt_sk[SK_MAX],acpt_sk[SK_MAX],pf_class;
	int sk1,clnt2_sk;
	int fd, err_no = 0;
	char filename[21];

        struct sockaddr_in conn_addr,lstn_addr,acpt_addr;

	/* Rather than fflush() throughout the code, set stdout to
         * be unbuffered.
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        pf_class = PF_INET;

        sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
        sk1 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*Creating a listen socket*/
        lstn_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*Creating a regular socket*/
	for (i = 0 ; i < SK_MAX ; i++)
		clnt_sk[i] = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	clnt2_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        conn_addr.sin_port = htons(SCTP_TESTPORT_1);

	lstn_addr.sin_family = AF_INET;
        lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	/*Binding the listen socket*/
	test_bind(lstn_sk, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

	/*Listening the socket*/
	test_listen(lstn_sk, SK_MAX-1);


	/*connect () TEST1: Bad socket descriptor, EBADF Expected error*/
	len = sizeof(struct sockaddr_in);
	error = connect(-1, (const struct sockaddr *) &conn_addr, len);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "connect with bad socket "
			 "descriptor error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() with bad socket descriptor - EBADF");
	
	/*connect () TEST2: Invalid socket, ENOTSOCK Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = connect(fd, (const struct sockaddr *) &conn_addr, len);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "connect with invalid socket "
                         "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "connect() with invalid socket - ENOTSOCK");

	/*connect () TEST3: Invalid address, EFAULT Expected error*/
	error = connect(sk, (const struct sockaddr *) -1, len);
	if (error != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "connect with invalid address "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() with invalid address - EFAULT");

	/*connect () TEST4: Invalid address length, EINVAL Expected error*/
	error = connect(sk, (const struct sockaddr *) &conn_addr, (len - 3));
	if (error != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "connect with invalid address length "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() with invalid address length - EINVAL");

	/*connect () TEST5: Invalid address family, EINVAL Expect error*/
	conn_addr.sin_family = 9090; /*Assigning invalid address family*/
        error = connect(sk, (const struct sockaddr *) &conn_addr, len);
        if (error != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "connect with invalid address family "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() with invalid address family - EINVAL");

	conn_addr.sin_family = AF_INET;

	/*connect () TEST6: Blocking connect, should pass*/
	/*All the be below blocking connect should pass as socket will be 
	listening SK_MAX clients*/
	for (i = 0 ; i < SK_MAX ; i++) {
		error = connect(clnt_sk[i], (const struct sockaddr *)&conn_addr,
			      len);
		if (error < 0)
			tst_brkm(TBROK, tst_exit, "valid blocking connect "
				 "error:%d, errno:%d", error, errno);
	}

	tst_resm(TPASS, "valid blocking connect() - SUCCESS");

	/*connect () TEST7: connect when accept queue is full, ECONNREFUSED
	Expect error*/
	/*Now that accept queue is full, the below connect should fail*/
	error = connect(clnt2_sk, (const struct sockaddr *) &conn_addr, len);
	if (error != -1 || errno != ECONNREFUSED)
		tst_brkm(TBROK, tst_exit, "connect when accept queue is full "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() when accept queue is full - ECONNREFUSED");
	
	/*Calling a accept first to estblish the pending connections*/
	for (i=0 ; i < SK_MAX ; i++)
		acpt_sk[i] = test_accept(lstn_sk,
					 (struct sockaddr *) &acpt_addr, &len);

	/*connect () TEST8: from a listening socket, EISCONN Expect error*/
	error = connect(lstn_sk, (const struct sockaddr *) &lstn_addr, len);
	if (error != -1 || errno != EISCONN)
		tst_brkm(TBROK, tst_exit, "connect on a listening socket "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() on a listening socket - EISCONN");

	/*connect() TEST9: On established socket, EISCONN Expect error*/
	i=0;
	error = connect(acpt_sk[i], (const struct sockaddr *) &lstn_addr, len);
        if (error != -1 || errno != EISCONN)
		tst_brkm(TBROK, tst_exit, "connect on an established socket "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() on an established socket - EISCONN");

	for (i = 0 ; i < 4 ; i++) {
		close(clnt_sk[i]);
		close(acpt_sk[i]);
	} 

	/* connect() TEST10: Re-establish an association that is closed.
	 * should succeed.
	 */
	error = connect(sk1, (const struct sockaddr *)&conn_addr, len);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "Re-establish an association that "
				 "is closed error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connect() to re-establish a closed association - "
		 "SUCCESS");

	close(sk);
	close(sk1);
	close(lstn_sk);

	return 0;
}
