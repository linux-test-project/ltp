/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2002, 2003
 * Copyright (c) 1999-2001 Motorola, Inc.
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
 * Written or modified by:
 *    Sridhar Samudrala		<sri@us.ibm.com>
 */

/* This is a kernel test to verify the one-to-many style sctp_connectx()
 * in blocking and non-blocking modes.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 9;
int TST_CNT = 0;

#define NUMADDR 6
#define SCTP_IP_LOOPBACK_I(I)  htonl(0x7f000001 + I)

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

int
main(int argc, char *argv[])
{
	int svr_sk, clt_sk1, clt_sk2, peeloff_sk;
	sctp_assoc_t associd, svr_associd1, svr_associd2, clt_associd1, clt_associd2;
	struct iovec iov;
	struct msghdr inmessage;
	int error, i;
	struct sctp_assoc_change *sac;
	char *big_buffer;
	int flags;
	struct sockaddr_in svr_loop[NUMADDR];
	struct sockaddr_in svr_try[NUMADDR];
	struct sockaddr_in clt_loop1[NUMADDR];
	struct sockaddr_in clt_loop2[NUMADDR];
	struct sockaddr_in clt_loop3[NUMADDR];
	sockaddr_storage_t svr_test[NUMADDR], clt_test1[NUMADDR], clt_test2[NUMADDR];

	/* Rather than fflush() throughout the code, set stdout to 
	 * be unbuffered.  
	 */ 
	setvbuf(stdout, NULL, _IONBF, 0); 

	for (i = 0; i < NUMADDR; i++) {
		/* Initialize the server and client addresses. */ 
		svr_loop[i].sin_family = AF_INET;
		svr_loop[i].sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i);
		svr_loop[i].sin_port = htons(SCTP_TESTPORT_1);
		svr_test[i].v4.sin_family = AF_INET;
		svr_test[i].v4.sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i);
		svr_test[i].v4.sin_port = htons(SCTP_TESTPORT_1);
		svr_try[i].sin_family = AF_INET;
		if (i < (NUMADDR-1)) {
			svr_try[i].sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i);
		} else {
			/* Make last address invalid. */
			svr_try[i].sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i + 0x400);
		}
		svr_try[i].sin_port = htons(SCTP_TESTPORT_1);
		clt_loop1[i].sin_family = AF_INET;
		clt_loop1[i].sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i + 0x100);
		clt_loop1[i].sin_port = htons(SCTP_TESTPORT_2);
		clt_test1[i].v4.sin_family = AF_INET;
		clt_test1[i].v4.sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i + 0x100);
		clt_test1[i].v4.sin_port = htons(SCTP_TESTPORT_2);
		clt_loop2[i].sin_family = AF_INET;
		clt_loop2[i].sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i + 0x200);
		clt_loop2[i].sin_port = htons(SCTP_TESTPORT_2+1);
		clt_test2[i].v4.sin_family = AF_INET;
		clt_test2[i].v4.sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i + 0x200);
		clt_test2[i].v4.sin_port = htons(SCTP_TESTPORT_2+1);
		clt_loop3[i].sin_family = AF_INET;
		clt_loop3[i].sin_addr.s_addr = SCTP_IP_LOOPBACK_I(i + 0x300);
		clt_loop3[i].sin_port = htons(SCTP_TESTPORT_2+2);
	}

	/* Create and bind the server socket.  */
	svr_sk = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	test_bind(svr_sk, (struct sockaddr *)&svr_loop[0], sizeof(svr_loop[0]));
	test_bindx_add(svr_sk, (struct sockaddr *)&svr_loop[1], NUMADDR-1);

	/* Mark server socket as being able to accept new associations.  */
	test_listen(svr_sk, 1);

	/* Create and bind the client sockets.  */
	clt_sk1 = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	test_bind(clt_sk1, (struct sockaddr *)&clt_loop1[0], sizeof(clt_loop1));
	test_bindx_add(clt_sk1, (struct sockaddr *)&clt_loop1[1], NUMADDR-1);
	clt_sk2 = test_socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	test_bind(clt_sk2, (struct sockaddr *)&clt_loop2[0], sizeof(clt_loop2));
	test_bindx_add(clt_sk2, (struct sockaddr *)&clt_loop2[1], NUMADDR-1);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(svr_sk);
	test_enable_assoc_change(clt_sk1);
	test_enable_assoc_change(clt_sk2);

	/* Set clt_sk1 as non-blocking. */
	flags = fcntl(clt_sk1, F_GETFL, 0);
	if (flags < 0)
		tst_brkm(TBROK, tst_exit, "fcntl F_GETFL: %s", strerror(errno));
	if (fcntl(clt_sk1, F_SETFL, flags | O_NONBLOCK) < 0)
		tst_brkm(TBROK, tst_exit, "fcntl F_SETFL: %s", strerror(errno));

	/* Do a non-blocking connectx from clt_sk1 to svr_sk */      
	error = sctp_connectx(clt_sk1, (struct sockaddr *)svr_try, NUMADDR,
			      &associd);
	/* Non-blocking connectx should return immediately with EINPROGRESS. */
	if ((error != -1) || (EINPROGRESS != errno))
		tst_brkm(TBROK, tst_exit, "non-blocking connectx error: %d"
			 "errno:%d", error, errno);

	tst_resm(TPASS, "non-blocking connectx");

	/* Doing a connectx on a socket to create an association that is
	 * is already established should return EISCONN.
	 */
	error = sctp_connectx(clt_sk1, (struct sockaddr *)svr_try, NUMADDR,
			      NULL);
	if ((error != -1) || (EISCONN != errno))
		tst_brkm(TBROK, tst_exit, "connectx on a socket to create an "
			 "assoc that is already established error:%d errno:%d",
			 error, errno);

	tst_resm(TPASS, "connectx on a socket to create an assoc that is "
		 "already established");

	/* Initialize inmessage for all receives. */
	memset(&inmessage, 0, sizeof(inmessage));
	big_buffer = test_malloc(REALLY_BIG);
	iov.iov_base = big_buffer;
	iov.iov_len = REALLY_BIG;
	inmessage.msg_iov = &iov;
	inmessage.msg_iovlen = 1;
	inmessage.msg_control = NULL;

	/* Get COMM_UP on clt_sk1 */
	error = test_recvmsg(clt_sk1, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	clt_associd1 = sac->sac_assoc_id;

	if (associd) {
		if (associd != clt_associd1)
			tst_brkm(TBROK, tst_exit, "Association id mismatch: "
			 "connectx returned %d, notification returned:%d",
			 associd, clt_associd1);
		tst_resm(TPASS, "Association id match between sctp_connectx()"
				" and notification.");
	}

	/* Get COMM_UP on svr_sk */
	error = test_recvmsg(svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	svr_associd1 = sac->sac_assoc_id;

	/* Do a blocking connectx from clt_sk2 to svr_sk. 
	 * Blocking connectx should block until the association is established
	 * and return success.
	 */
	test_connectx(clt_sk2, (struct sockaddr *)svr_try, NUMADDR);

	/* Get COMM_UP on clt_sk2 */
	error = test_recvmsg(clt_sk2, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	clt_associd2 = sac->sac_assoc_id;

	/* Get COMM_UP on svr_sk */
	error = test_recvmsg(svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	svr_associd2 = sac->sac_assoc_id;

	tst_resm(TPASS, "blocking connectx");

	peeloff_sk = test_sctp_peeloff(svr_sk, svr_associd1); 

	/* Doing a connectx on a peeled off socket should fail. */
	error = sctp_connectx(peeloff_sk, (struct sockaddr *)clt_loop3, NUMADDR,
			      NULL);
	if ((error != -1) || (EISCONN != errno))
		tst_brkm(TBROK, tst_exit, "connectx on a peeled off socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "connectx on a peeled off socket");

	/* Trying to create an association on a socket that matches an 
	 * existing peeled-off association should fail.
	 */
	error = sctp_connectx(svr_sk, (struct sockaddr *)clt_loop1, NUMADDR,
			      NULL);
	if ((error != -1) || (EADDRNOTAVAIL != errno))
		tst_brkm(TBROK, tst_exit, "connectx to create an assoc that "
			 "matches a peeled off assoc error:%d errno:%d",
			 error, errno);

	tst_resm(TPASS, "connectx to create an assoc that matches a peeled off "
		 "assoc");

	test_peer_addr(peeloff_sk, svr_associd1, clt_test1, NUMADDR);
	tst_resm(TPASS, "server association 1 peers ok");
	test_peer_addr(svr_sk, svr_associd2, clt_test2, NUMADDR);
	tst_resm(TPASS, "server association 2 peers ok");
	test_peer_addr(clt_sk1, clt_associd1, svr_test, NUMADDR);
	tst_resm(TPASS, "client association 1 peers ok");
	test_peer_addr(clt_sk2, clt_associd2, svr_test, NUMADDR);
	tst_resm(TPASS, "client association 2 peers ok");
	close(svr_sk);
	close(clt_sk1);
	close(clt_sk2);
	close(peeloff_sk);

	/* Indicate successful completion.  */
	return 0; 
}
