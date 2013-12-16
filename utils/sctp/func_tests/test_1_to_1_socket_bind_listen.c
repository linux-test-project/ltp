/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file has test cases to test the socket (), bind () and listen () for
 * 1-1 style sockets
 *
 * socket () Tests:
 * ---------------
 * TEST1: Invalid domain
 * TEST2: Invalid type
 * TEST3: Opening a TCP style socket
 *
 * bind () Tests:
 * -------------
 * TEST4: Invalid address
 * TEST5: Invalid address length
 * TEST6: Invalid socket descriptor
 * TEST7: Invalid host name
 * TEST8: On a socket that is already bound
 * TEST9: On reserved ports
 * TEST10: INADDR_ANY address and non-zero port
 * TEST11: INADDR_ANY address and zero port
 * TEST12: Local address and zero port
 * 
 * listen () Tests:
 * ---------------
 * TEST13: Bad socket descriptor
 * TEST14: Invalid socket
 * TEST15: Listening a bound socket
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

#define SCTP_RESERVED_PORT 7
#define SCTP_INV_LOOPBACK "172.31.43.112"

char *TCID = __FILE__;
int TST_TOTAL = 15;
int TST_CNT = 0;

int
main(int argc, char *argv[])
{
        int sk,pf_class;
	int error = 0;
	int uid;
	int fd, err_no = 0;
	char filename[21];

        struct sockaddr_in bind_addr;

	/* Rather than fflush() throughout the code, set stdout to
         * be unbuffered.
         */
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        pf_class = PF_INET;

        /* socket() TEST1: Invalid domain, EAFNOSUPPORT Expected error */
        sk = socket(-1, SOCK_STREAM, IPPROTO_SCTP);
        if (sk != -1 || errno != EAFNOSUPPORT)
		tst_brkm(TBROK, tst_exit, "socket() with invalid domain "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "socket() with invalid domain - EAFNOSUPPORT");

	/*socket() TEST2 : Invalid type, EINVAL Expected error*/
        sk = socket(pf_class, -1, IPPROTO_SCTP);
        if (sk != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "socket() with invalid type "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "socket() with invalid type - EINVAL");

	/*socket() TEST3: opening a socket*/
        sk = socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
        if (sk < 0)
		tst_brkm(TBROK, tst_exit, "valid socket() call "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "socket() - SUCCESS");

	/*bind() TEST4: Invalid structure, EFAULT Expected error */
        error = bind(sk, (struct sockaddr *)-1, sizeof(struct sockaddr_in));
        if (error != -1 || errno != EFAULT)
		tst_brkm(TBROK, tst_exit, "bind() with invalid address ptr "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "bind() with invalid address ptr - EFAULT");

	/*bind() TEST5: Invalid address length, EINVAL Expect error*/
	bind_addr.sin_family = AF_INET;
        bind_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        bind_addr.sin_port = htons(SCTP_TESTPORT_1);

	error = bind(sk, (struct sockaddr *) &bind_addr, sizeof(bind_addr)-2);
        if (error != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "bind() with invalid address length "
                         "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "bind() with invalid address length - EINVAL");

	/*bind() TEST6: Invalid socket descriptor, ENOTSOCK Expect Error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = bind(fd, (struct sockaddr *) &bind_addr, sizeof(bind_addr));
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "bind() with invalid socket "
			 "descriptor error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "bind() with invalid socket descriptor - ENOTSOCK");

	/*bind() TEST7: Invalid host name, EADDRNOTAVAIL Expect Error*/
	/*Assigning invalid host name*/
	bind_addr.sin_addr.s_addr = inet_addr(SCTP_INV_LOOPBACK);
	error = bind(sk, (struct sockaddr *) &bind_addr, sizeof(bind_addr));
        if (error != -1 || errno != EADDRNOTAVAIL)
		tst_brkm(TBROK, tst_exit, "bind() with invalid local "
			 "address error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "bind() with invalid local address - EADDRNOTAVAIL");

	/*bind() TEST8: Bind on a socket that has already called bind
	EINAVL, Expected error*/
	bind_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	/*Calling bind first time, it should pass*/
	test_bind(sk, (struct sockaddr *) &bind_addr, sizeof(bind_addr));

	error = bind(sk, (struct sockaddr *) &bind_addr, sizeof(bind_addr));
	if (error != -1 || errno != EINVAL)
		tst_brkm(TBROK, tst_exit, "bind() on an already bound socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "bind() on an already bound socket - EINVAL");

	/*Closing the socket which succeed in bind() */
	close(sk);

	/*Opening the socket again for further test*/
	sk = socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*bind() TEST9: Bind on reserved ports EACCES, Expected error*/
	/*Assigning a reserved port*/
	uid = getuid();
	if (uid != 0) {
		bind_addr.sin_port = htons(SCTP_RESERVED_PORT);	
		error = bind(sk, (struct sockaddr *) &bind_addr,
			     sizeof(bind_addr));
		if (error != -1 || errno != EACCES)
			tst_brkm(TBROK, tst_exit, "bind() on reserverd port "
			 "error:%d, errno:%d", error, errno);

		tst_resm(TPASS, "bind() on reserved port - EACCESS");
	}

	/*bind() TEST10: INADDR_ANY address and non-zero port, bind() should 
	succeed*/
	bind_addr.sin_addr.s_addr = INADDR_ANY;
        bind_addr.sin_port = htons(SCTP_TESTPORT_1);
	error = bind(sk, (struct sockaddr *) &bind_addr,sizeof(bind_addr));
	if ( error < 0 )
		tst_brkm(TBROK, tst_exit, "bind() with INADDR_ANY address and "
			 "non-zero port error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "bind() with INADDR_ANY address and non-zero port - "
		 "SUCCESS");
	
	/*Closing the socket which succeed in bind() */
	close(sk);

	/*Opening the socket again for further test*/
	sk = socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*bind() TEST11: INADDR_ANY address and zero port, bind() should 
	succeed*/
        bind_addr.sin_port = 0;
	error = bind(sk, (struct sockaddr *) &bind_addr,sizeof(bind_addr));
	if ( error < 0 )
		tst_brkm(TBROK, tst_exit, "bind() with INADDR_ANY address and "
			 "zero port error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "bind() with INADDR_ANY address and zero port - "
		 "SUCCESS");

	/*Closing the socket which succeed in bind() */
	close(sk);

	/*Opening the socket again for further test*/
	sk = socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/*bind() TEST12: local address and zero port, bind() should 
	succeed*/
        bind_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        bind_addr.sin_port = 0;
	error = bind(sk, (struct sockaddr *) &bind_addr,sizeof(bind_addr));
	if ( error < 0 )
		tst_brkm(TBROK, tst_exit, "bind() with local address and "
			 "zero port error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "bind() with local address and zero port - "
		 "SUCCESS");

	/*listen() TEST13: Bad socket descriptor EBADF, Expected error*/
	error = listen(-1, 3);
	if (error != -1 || errno != EBADF)
		tst_brkm(TBROK, tst_exit, "listen() with bad socket descriptor "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "listen() with bad socket descriptor - EBADF");

	/*listen() TEST14: Invalid socket ENOTSOCK, Expected error*/
	strcpy(filename, "/tmp/sctptest.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1)
		tst_brkm(TBROK, tst_exit, "Failed to mkstemp %s: %s",
				filename, strerror(errno));
	error = listen(fd, 3);
	if (error == -1)
		err_no = errno;
	close(fd);
	unlink(filename);
	if (error != -1 || err_no != ENOTSOCK)
		tst_brkm(TBROK, tst_exit, "listen() with invalid socket "
			 "error:%d, errno:%d", error, err_no);

	tst_resm(TPASS, "listen() with invalid socket - ENOTSOCK");

	/*listen() TEST15:listen on a bound socket, should succeed*/
	error = listen(sk, 3);
	if ( error < 0 )
		tst_brkm(TBROK, tst_exit, "listen() on a bound socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "listen() on a bound socket - SUCCESS");

	close(sk);
	
	return 0;
}
