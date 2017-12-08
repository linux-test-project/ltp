/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2003
 * Copyright (c) 2003 Intel Corp.
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
 * Any bugs reported to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release.
 *
 * Written or modified by:
 * To compile the v6 version, set the symbol TEST_V6 to 1.
 *
 * Written or modified by:
 *    Ardelle Fan		<ardelle.fan@intel.com>
 *    Sridhar Samudrala	 	<sri@us.ibm.com>
 */

/* This is a basic functional test for the SCTP new library APIs
 * sctp_sendmsg() and sctp_recvmsg(). test_timetolive.c is rewritten using
 * these new APIs. 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 10;
int TST_CNT = 0;

/* RCVBUF value, and indirectly RWND*2 */
#define SMALL_RCVBUF 3000
#define SMALL_MAXSEG 500
/* This is extra data length to ensure rwnd closes */
#define RWND_SLOP    100
static char *fillmsg = NULL;
static char *ttlmsg = "This should time out!\n";
static char *nottlmsg = "This should NOT time out!\n";
static char ttlfrag[SMALL_MAXSEG*3] = {0};
static char *message = "Hello world\n";

int main(int argc, char *argv[])
{
	int sk1, sk2;
	sockaddr_storage_t loop1;
	sockaddr_storage_t loop2;
	sockaddr_storage_t msgname;
	int error;
	int pf_class;
	uint32_t ppid;
	uint32_t stream;
	struct sctp_event_subscribe subscribe;
	char *big_buffer;
	int offset, msg_flags;
	socklen_t msgname_len;
	size_t buflen;
	struct sctp_send_failed *ssf;
	struct sctp_sndrcvinfo sinfo;
	struct sctp_sndrcvinfo snd_sinfo;
	sctp_assoc_t associd1;
	socklen_t len, oldlen;
	struct sctp_status gstatus;

	/* Rather than fflush() throughout the code, set stdout to
	 * be unbuffered.
	 */
	setvbuf(stdout, NULL, _IONBF, 0);

	/* Set some basic values which depend on the address family. */
#if TEST_V6
	pf_class = PF_INET6;

	loop1.v6.sin6_family = AF_INET6;
	loop1.v6.sin6_addr = in6addr_loopback;
	loop1.v6.sin6_port = htons(SCTP_TESTPORT_1);

	loop2.v6.sin6_family = AF_INET6;
	loop2.v6.sin6_addr = in6addr_loopback;
	loop2.v6.sin6_port = htons(SCTP_TESTPORT_2);
#else
	pf_class = PF_INET;

	loop1.v4.sin_family = AF_INET;
	loop1.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	loop1.v4.sin_port = htons(SCTP_TESTPORT_1);

	loop2.v4.sin_family = AF_INET;
	loop2.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	loop2.v4.sin_port = htons(SCTP_TESTPORT_2);
#endif /* TEST_V6 */

	/* Create the two endpoints which will talk to each other.  */
	sk1 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
        sk2 = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Set the MAXSEG to something smallish. */
	{
		int val = SMALL_MAXSEG;
		test_setsockopt(sk1, SCTP_MAXSEG, &val, sizeof(val));
	}

	memset(&subscribe, 0, sizeof(subscribe));
	subscribe.sctp_data_io_event = 1;
	subscribe.sctp_association_event = 1;
	subscribe.sctp_send_failure_event = 1;
	test_setsockopt(sk1, SCTP_EVENTS, &subscribe, sizeof(subscribe));
	test_setsockopt(sk2, SCTP_EVENTS, &subscribe, sizeof(subscribe));

        /* Bind these sockets to the test ports.  */
        test_bind(sk1, &loop1.sa, sizeof(loop1));
        test_bind(sk2, &loop2.sa, sizeof(loop2));

	/*
	 * Set the RWND small so we can fill it up easily.
	 * then reset RCVBUF to avoid frame droppage
	 */
	len = sizeof(int);
	error = getsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &oldlen, &len);
       
	if (error)
		tst_brkm(TBROK, tst_exit, "can't get rcvbuf size: %s",
			 strerror(errno));

	len = SMALL_RCVBUF; /* Really becomes 2xlen when set. */

	error = setsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));
	if (error)
		tst_brkm(TBROK, tst_exit, "setsockopt(SO_RCVBUF): %s",
			 strerror(errno));

       /* Mark sk2 as being able to accept new associations.  */
	test_listen(sk2, 1);

	/* Send the first message.  This will create the association.  */
	ppid = rand();
	stream = 1;
	test_sctp_sendmsg(sk1, message, strlen(message) + 1,
			  (struct sockaddr *)&loop2, sizeof(loop2),
			  ppid, 0, stream, 0, 0);

	tst_resm(TPASS, "sctp_sendmsg");

	/* Get the communication up message on sk2.  */
	buflen = REALLY_BIG;
	big_buffer = test_malloc(buflen);
	msgname_len = sizeof(msgname);
	msg_flags = 0;
	error = test_sctp_recvmsg(sk2, big_buffer, buflen,
				  (struct sockaddr *)&msgname, &msgname_len,
				  &sinfo, &msg_flags);
#if 0
	associd2 = ((struct sctp_assoc_change *)big_buffer)->sac_assoc_id;
#endif
	test_check_buf_notification(big_buffer, error, msg_flags,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);


	/* restore the rcvbuffer size for the receiving socket */
	error = setsockopt(sk2, SOL_SOCKET, SO_RCVBUF, &oldlen,
			   sizeof(oldlen));

	if (error)
		tst_brkm(TBROK, tst_exit, "setsockopt(SO_RCVBUF): %s",
			 strerror(errno));

	/* Get the communication up message on sk1.  */
	buflen = REALLY_BIG;
	msgname_len = sizeof(msgname);
	msg_flags = 0;
	error = test_sctp_recvmsg(sk1, big_buffer, buflen,
				  (struct sockaddr *)&msgname, &msgname_len,
				  &sinfo, &msg_flags); 
	associd1 = ((struct sctp_assoc_change *)big_buffer)->sac_assoc_id;
	test_check_buf_notification(big_buffer, error, msg_flags,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);

	tst_resm(TPASS, "sctp_recvmsg SCTP_COMM_UP notification");

	/* Get the first message which was sent.  */
	buflen = REALLY_BIG;
	msgname_len = sizeof(msgname);
	msg_flags = 0;
	error = test_sctp_recvmsg(sk2, big_buffer, buflen,
				  (struct sockaddr *)&msgname, &msgname_len,
				  &sinfo, &msg_flags); 
	test_check_buf_data(big_buffer, error, msg_flags, &sinfo,
			    strlen(message) + 1, MSG_EOR, stream, ppid); 

	tst_resm(TPASS, "sctp_recvmsg data");

	/* Figure out how big to make our fillmsg */
	len = sizeof(struct sctp_status);
	memset(&gstatus,0,sizeof(struct sctp_status));
	gstatus.sstat_assoc_id = associd1;
	error = getsockopt(sk1, IPPROTO_SCTP, SCTP_STATUS, &gstatus, &len);

	if (error)
		tst_brkm(TBROK, tst_exit, "can't get rwnd size: %s",
			strerror(errno));
	tst_resm(TINFO, "creating a fillmsg of size %d",
		gstatus.sstat_rwnd+RWND_SLOP);
        fillmsg = malloc(gstatus.sstat_rwnd+RWND_SLOP);

	/* Send a fillmsg */
	memset(fillmsg, 'X', gstatus.sstat_rwnd+RWND_SLOP);
	fillmsg[gstatus.sstat_rwnd+RWND_SLOP-1] = '\0';
	ppid++;
	stream++;
	test_sctp_sendmsg(sk1, fillmsg, gstatus.sstat_rwnd+RWND_SLOP, 
			  (struct sockaddr *)&loop2, sizeof(loop2),
			  ppid, 0, stream, 0, 0);

	/* Now send a message that will timeout. */
	test_sctp_sendmsg(sk1, ttlmsg, strlen(ttlmsg) + 1,
			  (struct sockaddr *)&loop2, sizeof(loop2),
			  ppid, 0, stream, 2000, 0);

	tst_resm(TPASS, "sctp_sendmsg with ttl");

	/* Next send a message that won't time out. */
	test_sctp_sendmsg(sk1, nottlmsg, strlen(nottlmsg) + 1,
			  (struct sockaddr *)&loop2, sizeof(loop2),
			  ppid, 0, stream, 0, 0);

	tst_resm(TPASS, "sctp_sendmsg with zero ttl");

	/* And finally a fragmented message that will time out. */
	memset(ttlfrag, '0', sizeof(ttlfrag));
	ttlfrag[sizeof(ttlfrag)-1] = '\0';
	test_sctp_sendmsg(sk1, ttlfrag, sizeof(ttlfrag),
			  (struct sockaddr *)&loop2, sizeof(loop2),
			  ppid, 0, stream, 2000, 0);

	tst_resm(TPASS, "sctp_sendmsg fragmented msg with ttl");

	/* Sleep waiting for the message to time out. */
	tst_resm(TINFO, "**  SLEEPING for 3 seconds **");
	sleep(3);

	/* Get the fillmsg. */
	do {
		buflen = REALLY_BIG;
		msgname_len = sizeof(msgname);
		msg_flags = 0;
		test_sctp_recvmsg(sk2, big_buffer, buflen,
			  (struct sockaddr *)&msgname, &msgname_len,
			  &sinfo, &msg_flags); 
	} while (!(msg_flags & MSG_EOR));

	/* Get the message that did NOT time out. */
	buflen = REALLY_BIG;
	msgname_len = sizeof(msgname);
	msg_flags = 0;
	error = test_sctp_recvmsg(sk2, big_buffer, buflen,
			  (struct sockaddr *)&msgname, &msgname_len,
			  &sinfo, &msg_flags); 
	test_check_buf_data(big_buffer, error, msg_flags, &sinfo,
			    strlen(nottlmsg) + 1, MSG_EOR, stream, ppid); 
	if (0 != strncmp(big_buffer, nottlmsg, strlen(nottlmsg)))
		tst_brkm(TBROK, tst_exit, "sctp_recvmsg: Wrong Message !!!");

	tst_resm(TPASS, "sctp_recvmsg msg with zero ttl");

	/* Get the SEND_FAILED notification for the message that DID
	 * time out.
	 */
	buflen = REALLY_BIG;
	msgname_len = sizeof(msgname);
	msg_flags = 0;
	error = test_sctp_recvmsg(sk1, big_buffer, buflen,
			  (struct sockaddr *)&msgname, &msgname_len,
			  &sinfo, &msg_flags); 
	test_check_buf_notification(big_buffer, error, msg_flags,
				    sizeof(struct sctp_send_failed) +
							strlen(ttlmsg) + 1,
				    SCTP_SEND_FAILED, 0);
	ssf = (struct sctp_send_failed *)big_buffer;
	if (0 != strncmp(ttlmsg, (char *)ssf->ssf_data, strlen(ttlmsg) + 1))
		tst_brkm(TBROK, tst_exit, "SEND_FAILED data mismatch");

	tst_resm(TPASS, "sctp_recvmsg SEND_FAILED for message with ttl");

	offset = 0;

	/* Get the SEND_FAILED notifications for the fragmented message that 
	 * timed out.
	 */
	do {
		buflen = REALLY_BIG;
		msgname_len = sizeof(msgname);
		msg_flags = 0;
		error = test_sctp_recvmsg(sk1, big_buffer, buflen,
			  (struct sockaddr *)&msgname, &msgname_len,
			  &sinfo, &msg_flags); 
		test_check_buf_notification(big_buffer, error, msg_flags,
					    sizeof(struct sctp_send_failed) +
							          SMALL_MAXSEG,
					    SCTP_SEND_FAILED, 0);
		ssf = (struct sctp_send_failed *)big_buffer;
		if (0 != strncmp(&ttlfrag[offset], (char *)ssf->ssf_data,
				 SMALL_MAXSEG))
			tst_brkm(TBROK, tst_exit, "SEND_FAILED data mismatch");
		offset += SMALL_MAXSEG;
	} while (!(ssf->ssf_info.sinfo_flags & 0x01)); /* LAST FRAG */

	tst_resm(TPASS, "sctp_recvmsg SEND_FAILED for fragmented message with "
		 "ttl");

	snd_sinfo.sinfo_ppid = rand();
	snd_sinfo.sinfo_flags = 0; 
	snd_sinfo.sinfo_stream = 2; 
	snd_sinfo.sinfo_timetolive = 0; 
	snd_sinfo.sinfo_assoc_id = associd1; 
	test_sctp_send(sk1, message, strlen(message) + 1, &snd_sinfo,
		       MSG_NOSIGNAL);

	buflen = REALLY_BIG;
	msgname_len = sizeof(msgname);
	msg_flags = 0;
	error = test_sctp_recvmsg(sk2, big_buffer, buflen,
				  (struct sockaddr *)&msgname, &msgname_len,
				  &sinfo, &msg_flags); 
	test_check_buf_data(big_buffer, error, msg_flags, &sinfo,
			    strlen(message) + 1, MSG_EOR, snd_sinfo.sinfo_stream,
			    snd_sinfo.sinfo_ppid); 

	tst_resm(TPASS, "sctp_send");

	/* Shut down the link.  */
	close(sk1);

	/* Get the shutdown complete notification. */
	buflen = REALLY_BIG;
	msgname_len = sizeof(msgname);
	msg_flags = 0;
	error = test_sctp_recvmsg(sk2, big_buffer, buflen,
		  (struct sockaddr *)&msgname, &msgname_len,
		  &sinfo, &msg_flags); 
	test_check_buf_notification(big_buffer, error, msg_flags,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);

	close(sk2);

	/* Indicate successful completion.  */
	return 0;	
}
