/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * When init timeout is set to zero, a connect () crashed the system. This case
 * tests the fix for the same.
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
int TST_TOTAL = 1;
int TST_CNT = 0;

int 
main (int argc, char **argv)
{
	int sk1, sk2, sk3, pf_class;
	socklen_t len;
	struct sockaddr_in lstn_addr, acpt_addr;
	struct sockaddr_in conn_addr;
	char * buffer_rcv;
	struct sctp_initmsg sinmsg;
	char *message = "Hello World!\n";

	/* Rather than fflush() throughout the code, set stdout to
	 * be unbuffered.
	 */
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	/* Opening the socket*/
	
	pf_class = PF_INET;

	sk1 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
	sk3 = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

        conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        conn_addr.sin_port = htons(SCTP_TESTPORT_1);

        lstn_addr.sin_family = AF_INET;
        lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	test_bind(sk3, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

	len = sizeof(struct sctp_initmsg);
	sinmsg.sinit_num_ostreams = 65535;
	sinmsg.sinit_max_instreams = 10;
	sinmsg.sinit_max_attempts = 1;
	sinmsg.sinit_max_init_timeo = 0;
	test_setsockopt(sk1, SCTP_INITMSG, &sinmsg, len);
	sinmsg.sinit_num_ostreams = 10;
	sinmsg.sinit_max_instreams = 65535;
	test_setsockopt(sk3, SCTP_INITMSG, &sinmsg, len);

	test_listen(sk3, 1);

	len = sizeof(struct sockaddr_in);
	test_connect(sk1, (struct sockaddr *) &conn_addr, len);

	sk2 = test_accept(sk3, (struct sockaddr *) &acpt_addr, &len);

	test_sctp_sendmsg(sk1, message, strlen(message) + 1,
			  (struct sockaddr *)&conn_addr, len,
			  0, 0, 65534, 0, 0);

	buffer_rcv = malloc(100);
	test_recv(sk2, buffer_rcv, (strlen(message) + 1), MSG_NOSIGNAL);

	tst_resm(TPASS, "connect() with init timeout set to 0 - SUCCESS");

	close (sk1);
	close (sk2);
	close (sk3);
	
        return 0;
}
