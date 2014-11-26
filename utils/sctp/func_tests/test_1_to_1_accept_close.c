/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the accept () and close () call for 
 * 1-1 style sockets
 *
 * accept () Tests:
 * ---------------
 * TEST1: Bad socket descriptor
 * TEST2: Invalid socket
 * TEST3: Invalid address
 * TEST4: On a non-listening socket
 * TEST5: On a established socket
 * TEST6: On a CLOSED association
 * TEST7: Extracting the association on the listening socket
 *
 * close () Tests:
 * --------------
 * TEST8: Bad socket descriptor
 * TEST9: valid socket descriptor
 * TEST10: Closed socket descriptor
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

#define SK_MAX  10

int
main(int argc, char *argv[])
{
        socklen_t len;
	int i;
	int sk,lstn_sk,clnt_sk[SK_MAX],acpt_sk,pf_class;
	int new_sk[SK_MAX],clnt2_sk[SK_MAX];
	int error;
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

	for (i=0 ; i < SK_MAX ; i++)
		new_sk[i] = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/* Creating a regular socket */
	for (i = 0 ; i < SK_MAX ; i++)
		clnt_sk[i] = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	for (i = 0 ; i < SK_MAX ; i++)
		clnt2_sk[i] = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/* Creating a listen socket */
        lstn_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        conn_addr.sin_port = htons(SCTP_TESTPORT_1);

	lstn_addr.sin_family = AF_INET;
        lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	/* Binding the listen socket */
	test_bind(lstn_sk, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

	/* Listening many sockets as we are calling too many connect here */
	test_listen(lstn_sk, SK_MAX );

	/* connect() is called just to make sure accept() doesn't block the
	 * program
	 */
	i = 0;
	len = sizeof(struct sockaddr_in);
	test_connect(clnt_sk[i++], (struct sockaddr *) &conn_addr, len);

	/* accept() TEST1: Bad socket descriptor EBADF, Expected error */
        error = accept(-1, (struct sockaddr *) &acpt_addr, &len);
        if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "accept with a bad socket descriptor"
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "accept() with a bad socket descriptor - EBADF");

        /*accept() TEST2: Invalid socket ENOTSOCK, Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = accept(fd, (struct sockaddr *) &acpt_addr, &len);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "accept with invalid socket"
                         "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "accept() with invalid socket - ENOTSOCK");

        /*accept() TEST3: Invalid address EFAULT, Expected error*/
        error = accept(lstn_sk, (struct sockaddr *) -1, &len);
        if (error != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "accept with invalid address"
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "accept() with invalid address - EFAULT");

	test_connect(clnt_sk[i++], (struct sockaddr *) &conn_addr, len);

        /*accept() TEST4: on a non-listening socket EINVAL, Expected error*/
        error = accept(sk, (struct sockaddr *) &acpt_addr, &len);
        if (error != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "accept on a non-listening socket"
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "accept() on a non-listening socket - EINVAL");
	
	test_connect(clnt_sk[i++], (struct sockaddr *) &conn_addr, len);

	/*Calling accept to establish the connection*/
	acpt_sk = test_accept(lstn_sk, (struct sockaddr *) &acpt_addr, &len);

	/*accept() TEST5: On a established socket EINVAL, Expected error*/
	error = accept(acpt_sk, (struct sockaddr *) &acpt_addr, &len);
	if (error != -1 || (errno != EINVAL && errno != EACCES)) {
		tst_brkm(TBROK, tst_exit, "accept on an established socket"
		         "error:%d, errno:%d", error, errno);
	}

	tst_resm(TPASS, "accept() on an established socket - %s",
		tst_strerrno(errno));

	/*Closing the previously established association*/
	close(acpt_sk);

	test_connect(clnt_sk[i], (struct sockaddr *) &conn_addr, len);

	/*accept() TEST6: On the CLOSED association should succeed*/
	acpt_sk = accept(lstn_sk, (struct sockaddr *) &acpt_addr, &len);
        if (acpt_sk < 0)
		tst_brkm(TBROK, tst_exit, "accept a closed association"
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "accept() a closed association - SUCCESS");

	close(acpt_sk);

	/*accept() TEST7: Extracting the association on the listening socket
	as new socket, new socket socket descriptor should return*/
	for (i = 0 ; i < (SK_MAX - 1); i++)
		test_connect(clnt2_sk[i], (struct sockaddr *) &conn_addr, len);

	for (i = 0 ; i < (SK_MAX - 1); i++)
		new_sk[i] = test_accept(lstn_sk, (struct sockaddr *)&acpt_addr,
					&len);

	tst_resm(TPASS, "accept() on a listening socket - SUCCESS");

	
        /*close() TEST8: Bad socket descriptor, EBADF Expected error*/
	error = close(-1);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "close with a bad socket descriptor "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "close() with a bad socket descriptor - EBADF");

	/*close() TEST9: valid socket descriptor should succeed*/
	error = close(sk);
	if (error < 0)
		tst_brkm(TBROK, tst_exit, "close with a valid socket descriptor"
                         " error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "close() with a valid socket descriptor - SUCCESS");

	/*close() TEST10: closed socket descriptor, EBADF Expected error*/
        error = close(sk);
        if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "close with a closed socket "
			 "descriptor error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "close() with a closed socket descriptor - EBADF");
	
	for (i = 0 ; i < SK_MAX ; i++) {
		close(clnt_sk[i]);
		close(new_sk[i]);
		close(clnt2_sk[i]);
	}

	return 0;
}
