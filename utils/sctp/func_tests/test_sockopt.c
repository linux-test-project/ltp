/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2001, 2004
 * Copyright (c) 1999-2000 Cisco, Inc.
 * Copyright (c) 1999-2001 Motorola, Inc.
 * Copyright (c) 2001 Intel Corp.
 * Copyright (c) 2001 Nokia, Inc.
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
 *    La Monte H.P. Yarroll <piggy@acm.org>
 *    Karl Knutson <karl@athena.chicago.il.us>
 *    Hui Huang <hui.huang@nokia.com>
 *    Jon Grimm <jgrimm@us.ibm.com>
 *    Sridhar Samudrala <samudrala@us.ibm.com>
 */

/* This is a functional test to verify the various SCTP level socket
 * options that can be used to get information about existing SCTP
 * associations and to configure certain parameters.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sctputil.h>

char *TCID = __FILE__;
int TST_TOTAL = 29;
int TST_CNT = 0;

int
main(void)
{
	int udp_svr_sk, udp_clt_sk, tcp_svr_sk, tcp_clt_sk;
	int accept_sk, peeloff_sk;
        sockaddr_storage_t udp_svr_loop, udp_clt_loop;
        sockaddr_storage_t tcp_svr_loop, tcp_clt_loop;
        struct iovec iov;
        struct msghdr inmessage;
	struct msghdr outmessage;
	char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
	char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sinfo;
        struct iovec out_iov;
        char *message = "hello, world!\n";
        int error;
	int pf_class;
	uint32_t ppid;
	uint32_t stream;
	sctp_assoc_t udp_svr_associd, udp_clt_associd;
	struct sctp_assoc_change *sac;
	char *big_buffer;
	struct sctp_event_subscribe subscribe;
	struct sctp_initmsg initmsg;
	struct sctp_paddrparams paddrparams;
	struct sctp_sndrcvinfo set_udp_sk_dflt_param, get_udp_sk_dflt_param; 
	struct sctp_sndrcvinfo set_tcp_sk_dflt_param, get_tcp_sk_dflt_param; 
	struct sctp_sndrcvinfo set_udp_assoc_dflt_param;
	struct sctp_sndrcvinfo get_udp_assoc_dflt_param; 
	struct sctp_sndrcvinfo set_tcp_assoc_dflt_param;
	struct sctp_sndrcvinfo get_tcp_assoc_dflt_param; 
	struct sctp_sndrcvinfo get_peeloff_assoc_dflt_param; 
	struct sctp_sndrcvinfo get_accept_assoc_dflt_param; 
	struct sctp_paddrinfo pinfo;
	int dflt_pathmaxrxt;
	socklen_t optlen, addrlen;
	struct sctp_status status;
	struct sctp_assoc_value value;

        /* Rather than fflush() throughout the code, set stdout to
	 * be unbuffered.
	 */
	setvbuf(stdout, NULL, _IONBF, 0);

	/* Set some basic values which depend on the address family. */
#if TEST_V6
	pf_class = PF_INET6;

        udp_svr_loop.v6.sin6_family = AF_INET6;
        udp_svr_loop.v6.sin6_addr = in6addr_loopback;
        udp_svr_loop.v6.sin6_port = htons(SCTP_TESTPORT_1);

        udp_clt_loop.v6.sin6_family = AF_INET6;
        udp_clt_loop.v6.sin6_addr = in6addr_loopback;
        udp_clt_loop.v6.sin6_port = htons(SCTP_TESTPORT_1+1);

        tcp_svr_loop.v6.sin6_family = AF_INET6;
        tcp_svr_loop.v6.sin6_addr = in6addr_loopback;
        tcp_svr_loop.v6.sin6_port = htons(SCTP_TESTPORT_1+2);

        tcp_clt_loop.v6.sin6_family = AF_INET6;
        tcp_clt_loop.v6.sin6_addr = in6addr_loopback;
        tcp_clt_loop.v6.sin6_port = htons(SCTP_TESTPORT_1+3);
#else
	pf_class = PF_INET;

        udp_svr_loop.v4.sin_family = AF_INET;
        udp_svr_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        udp_svr_loop.v4.sin_port = htons(SCTP_TESTPORT_1);

        udp_clt_loop.v4.sin_family = AF_INET;
        udp_clt_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        udp_clt_loop.v4.sin_port = htons(SCTP_TESTPORT_1+1);

        tcp_svr_loop.v4.sin_family = AF_INET;
        tcp_svr_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        tcp_svr_loop.v4.sin_port = htons(SCTP_TESTPORT_1+2);

        tcp_clt_loop.v4.sin_family = AF_INET;
        tcp_clt_loop.v4.sin_addr.s_addr = SCTP_IP_LOOPBACK;
        tcp_clt_loop.v4.sin_port = htons(SCTP_TESTPORT_2+3);
#endif /* TEST_V6 */

        /* Create the two endpoints which will talk to each other.  */
        udp_svr_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
        udp_clt_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(udp_svr_sk);
	test_enable_assoc_change(udp_clt_sk);

        /* Bind these sockets to the test ports.  */
        test_bind(udp_svr_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));
        test_bind(udp_clt_sk, &udp_clt_loop.sa, sizeof(udp_clt_loop));

       /* Mark udp_svr_sk as being able to accept new associations.  */
	test_listen(udp_svr_sk, 1);

	/* TEST #1: SCTP_STATUS socket option. */
	/* Make sure that SCTP_STATUS getsockopt on a socket with no
	 * association fails.
	 */
	optlen = sizeof(struct sctp_status);
	memset(&status, 0, optlen);
	error = getsockopt(udp_svr_sk, SOL_SCTP, SCTP_STATUS, &status,
			   &optlen);
	if ((error != -1) && (errno != EINVAL))
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_STATUS) on a "
			 "socket with no assoc error:%d errno:%d",
			 error, errno);

	tst_resm(TPASS, "getsockopt(SCTP_STATUS) on a socket with no assoc");

        /* Send the first message.  This will create the association.  */
        outmessage.msg_name = &udp_svr_loop;
        outmessage.msg_namelen = sizeof(udp_svr_loop);
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
	ppid = rand(); /* Choose an arbitrary value. */
	stream = 1;
	sinfo->sinfo_ppid = ppid;
	sinfo->sinfo_stream = stream;
        outmessage.msg_iov->iov_base = message;
        outmessage.msg_iov->iov_len = strlen(message) + 1;
        test_sendmsg(udp_clt_sk, &outmessage, 0, strlen(message)+1);

	/* Initialize inmessage for all receives. */
	big_buffer = test_malloc(REALLY_BIG);
        memset(&inmessage, 0, sizeof(inmessage));
        iov.iov_base = big_buffer;
        iov.iov_len = REALLY_BIG;
        inmessage.msg_iov = &iov;
        inmessage.msg_iovlen = 1;
        inmessage.msg_control = incmsg;

        /* Get the communication up message on udp_svr_sk.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(udp_svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	udp_svr_associd = sac->sac_assoc_id;

        /* Get the communication up message on udp_clt_sk.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(udp_clt_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	udp_clt_associd = sac->sac_assoc_id;

        /* Get the first message which was sent.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(udp_svr_sk, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(message) + 1,
			    MSG_EOR, stream, ppid);

	/* Get SCTP_STATUS for udp_clt_sk's given association. */
	optlen = sizeof(struct sctp_status);
	memset(&status, 0, optlen);
	status.sstat_assoc_id = udp_clt_associd;
	test_getsockopt(udp_clt_sk, SCTP_STATUS, &status, &optlen);

	tst_resm(TPASS, "getsockopt(SCTP_STATUS)");

	/* Make sure that SCTP_STATUS getsockopt with invalid associd fails. */
	optlen = sizeof(struct sctp_status);
	memset(&status, 0, optlen);
	status.sstat_assoc_id = udp_svr_associd;
	error = getsockopt(udp_clt_sk, SOL_SCTP, SCTP_STATUS, &status,
			   &optlen); 
	if ((error != -1) && (errno != EINVAL))
        	tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_STATUS) with "
			 "associd error: %d errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt(SCTP_STATUS) with invalid associd");

	/* Make sure that SCTP_STATUS getsockopt with NULL associd fails. */
	optlen = sizeof(struct sctp_status);
	memset(&status, 0, optlen);
	status.sstat_assoc_id = 0;
	error = getsockopt(udp_svr_sk, SOL_SCTP, SCTP_STATUS, &status,
			   &optlen);
	if ((error != -1) && (errno != EINVAL))
        	tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_STATUS) with "
			 "NULL associd error: %d errno:%d", error, errno);

	tst_resm(TPASS, "getsockopt(SCTP_STATUS) with NULL associd");

        /* Shut down the link.  */
        close(udp_clt_sk);

        /* Get the shutdown complete notification. */
	inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(udp_svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_SHUTDOWN_COMP);	
				
	error = 0;
        close(udp_svr_sk);

	/* TEST #2: SCTP_EVENTS socket option and SCTP_SHUTDOWN_EVENT
	 * notification.
	 */
        /* Create the two endpoints which will talk to each other.  */
	udp_svr_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
	udp_clt_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(udp_svr_sk);
	test_enable_assoc_change(udp_clt_sk);

	/* Bind these sockets to the test ports.  */
	test_bind(udp_svr_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));
	test_bind(udp_clt_sk, &udp_clt_loop.sa, sizeof(udp_clt_loop));

	/* Mark udp_svr_sk as being able to accept new associations.  */
	test_listen(udp_svr_sk, 1);

	/* Get the default events that are enabled on udp_svr_sk. */
	optlen = sizeof(subscribe);
	test_getsockopt(udp_svr_sk, SCTP_EVENTS, &subscribe, &optlen);

	/* Get the default events that are enabled on udp_clt_sk. */
	optlen = sizeof(subscribe);
	test_getsockopt(udp_clt_sk, SCTP_EVENTS, &subscribe, &optlen);

	tst_resm(TPASS, "getsockopt(SCTP_EVENTS)");

	/* Disable all the events on udp_svr_sk and udp_clt_sk. */
	memset(&subscribe, 0, sizeof(struct sctp_event_subscribe));
	test_setsockopt(udp_svr_sk, SCTP_EVENTS, &subscribe,
			sizeof(subscribe));
	test_setsockopt(udp_clt_sk, SCTP_EVENTS, &subscribe,
			sizeof(subscribe));

	tst_resm(TPASS, "setsockopt(SCTP_EVENTS)");

	/* Get the updated list of enabled events on udp_svr_sk and
	 * udp_clt_sk.
	 */
	optlen = sizeof(subscribe);
	test_getsockopt(udp_svr_sk, SCTP_EVENTS, &subscribe, &optlen);
	optlen = sizeof(subscribe);
	test_getsockopt(udp_clt_sk, SCTP_EVENTS, &subscribe, &optlen);

	/* Send a message.  This will create the association.  */
	outmessage.msg_iov->iov_base = message;
	outmessage.msg_iov->iov_len = strlen(message) + 1;
	test_sendmsg(udp_clt_sk, &outmessage, 0, strlen(message)+1);

	/* Get the message which was sent.  */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(udp_svr_sk, &inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, error, strlen(message) + 1,
			    MSG_EOR, 0, 0);
	/* Verify that we received the msg without any ancillary data. */
	if (inmessage.msg_controllen != 0)
		tst_brkm(TBROK, tst_exit, "Receive unexpected ancillary"
			 "data");

	/* Enable SCTP_SHUTDOWN_EVENTs on udp_svr_sk. */
	memset(&subscribe, 0, sizeof(struct sctp_event_subscribe));
	subscribe.sctp_shutdown_event = 1;
	test_setsockopt(udp_svr_sk, SCTP_EVENTS, &subscribe,
			sizeof(subscribe));

	error = 0;
        /* Shut down the link.  */
        close(udp_clt_sk);

	/* Get the SHUTDOWN_EVENT notification on udp_svr_sk. */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(udp_svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_shutdown_event),
				    SCTP_SHUTDOWN_EVENT, 0);	

	tst_resm(TPASS, "setsockopt(SCTP_EVENTS) - SCTP_SHUTDOWN_EVENT");
 
        close(udp_svr_sk);

	/* TEST #3: whether sctp_opt_info equals */
        /* Create the two endpoints which will talk to each other.  */
	udp_svr_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
	udp_clt_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(udp_svr_sk);
	test_enable_assoc_change(udp_clt_sk);

	/* Bind these sockets to the test ports.  */
	test_bind(udp_svr_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));
	test_bind(udp_clt_sk, &udp_clt_loop.sa, sizeof(udp_clt_loop));

	/* Mark udp_svr_sk as being able to accept new associations.  */
	test_listen(udp_svr_sk, 1);

        /* Send the first message.  This will create the association.  */
        outmessage.msg_name = &udp_svr_loop;
        outmessage.msg_namelen = sizeof(udp_svr_loop);
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
	ppid = rand(); /* Choose an arbitrary value. */
	stream = 1;
	sinfo->sinfo_ppid = ppid;
	sinfo->sinfo_stream = stream;
        outmessage.msg_iov->iov_base = message;
        outmessage.msg_iov->iov_len = strlen(message) + 1;
        test_sendmsg(udp_clt_sk, &outmessage, 0, strlen(message)+1);
	
        /* Get the communication up message on udp_clt_sk.  */
        inmessage.msg_controllen = sizeof(incmsg);
        error = test_recvmsg(udp_clt_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	udp_clt_associd = sac->sac_assoc_id;

	/* Compare the SCTP_STATUS result between sctp_opt_info and 
	 * getsockopt
	 */
	{
		struct sctp_status status1, status2;

		memset(&status1, 0, sizeof(status1));
		memset(&status2, 0, sizeof(status2));
		optlen = sizeof(struct sctp_status);

		/* Test SCTP_STATUS for udp_clt_sk's given association. */	
		error = sctp_opt_info(udp_clt_sk,udp_clt_associd,SCTP_STATUS,
				(char *)&status1, &optlen);
		if (error != 0)
	                tst_brkm(TBROK, tst_exit,
				 "sctp_opt_info(SCTP_STATUS): %s", 
				 strerror(errno));

		status2.sstat_assoc_id = udp_clt_associd;
		error = getsockopt(udp_clt_sk, IPPROTO_SCTP, SCTP_STATUS,
                		(char *)&status2, &optlen);
		if (error != 0)
	                tst_brkm(TBROK, tst_exit,
				 "getsockopt(SCTP_STATUS): %s", 
				 strerror(errno));
		if (strncmp((char *)&status1, (char *)&status2, optlen))
	                tst_brkm(TBROK, tst_exit, "sctp_opt_info(SCTP_STAUS)"
			       "doesn't match getsockopt(SCTP_STATUS)");

                tst_resm(TPASS, "sctp_opt_info(SCTP_STATUS)");
	}
	error = 0;
        /* Shut down the link.  */
        close(udp_svr_sk);
        close(udp_clt_sk);

	/* TEST #4: SCTP_INITMSG socket option. */
        /* Create a socket.  */
	udp_svr_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Bind this socket to the test port.  */
	test_bind(udp_svr_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(udp_svr_sk);

	/* Get the default parameters for association initialization. */
	optlen = sizeof(initmsg);
	test_getsockopt(udp_svr_sk, SCTP_INITMSG, &initmsg, &optlen);

	tst_resm(TPASS, "getsockopt(SCTP_INITMSG)");

	/* Change the parameters for association initialization. */
	initmsg.sinit_num_ostreams = 5;
	initmsg.sinit_max_instreams = 5;
	initmsg.sinit_max_attempts = 3;
	initmsg.sinit_max_init_timeo = 30;
	test_setsockopt(udp_svr_sk, SCTP_INITMSG, &initmsg, sizeof(initmsg));

	tst_resm(TPASS, "setsockopt(SCTP_INITMSG)");

	/* Get the updated parameters for association initialization. */
	optlen = sizeof(initmsg);
	test_getsockopt(udp_svr_sk, SCTP_INITMSG, &initmsg, &optlen);
	
	close(udp_svr_sk);

	/* TEST #5: SCTP_PEER_ADDR_PARAMS socket option. */
        /* Create a socket.  */
	udp_svr_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);

	/* Get the default parameters for this endpoint */
	optlen = sizeof(paddrparams);
	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_address.ss_family = AF_INET;
	test_getsockopt(udp_svr_sk, SCTP_PEER_ADDR_PARAMS, &paddrparams,
								&optlen);

	dflt_pathmaxrxt = paddrparams.spp_pathmaxrxt;
	tst_resm(TPASS, "getsockopt(SCTP_PEER_ADDR_PARAMS)");

	/* Change the default parameters for this endpoint (socket) */
	paddrparams.spp_hbinterval = 1000;
	paddrparams.spp_pathmaxrxt = dflt_pathmaxrxt+1;
	paddrparams.spp_sackdelay = 100;
	test_setsockopt(udp_svr_sk, SCTP_PEER_ADDR_PARAMS, &paddrparams,
							sizeof(paddrparams));

	paddrparams.spp_pathmaxrxt = 0;

	/* Get the updated default parameters for this endpoint. */
	optlen = sizeof(paddrparams);
	test_getsockopt(udp_svr_sk, SCTP_PEER_ADDR_PARAMS, &paddrparams,
								&optlen);
	if (paddrparams.spp_pathmaxrxt != dflt_pathmaxrxt+1)
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "mismatch");

	value.assoc_id = 0;
	optlen = sizeof(value);
	test_getsockopt(udp_svr_sk, SCTP_DELAYED_ACK_TIME, &value,
								&optlen);
	if (value.assoc_value != 100)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DELAYED_ACK_TIME) "
			 "mismatch");

	value.assoc_id    = 0;
	value.assoc_value = 250;
	test_setsockopt(udp_svr_sk, SCTP_DELAYED_ACK_TIME, &value,
							sizeof(value));
	optlen = sizeof(paddrparams);
	test_getsockopt(udp_svr_sk, SCTP_PEER_ADDR_PARAMS, &paddrparams,
								&optlen);
	if (paddrparams.spp_sackdelay != 250)
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_DELAYED_ACK_TIME) "
			 "mismatch");

	tst_resm(TPASS, "setsockopt(SCTP_DELAYED_ACK_TIME)");


	/* Ensure that prior defaults are preserved for a new endpoint */
	udp_clt_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
	optlen = sizeof(paddrparams);
	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_address.ss_family = AF_INET;
	test_getsockopt(udp_clt_sk, SCTP_PEER_ADDR_PARAMS, &paddrparams,
								&optlen);
	if (paddrparams.spp_pathmaxrxt != dflt_pathmaxrxt)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "mismatch");

	
	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS)");

       	/* Invalid assoc id */
	paddrparams.spp_assoc_id = 1234;
        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid associd error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid associd");

	test_bind(udp_svr_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));
	test_bind(udp_clt_sk, &udp_clt_loop.sa, sizeof(udp_clt_loop));

	test_listen(udp_svr_sk, 5);

	test_enable_assoc_change(udp_svr_sk);
	test_enable_assoc_change(udp_clt_sk);

	/* Do a connect on a UDP-style socket and establish an association. */
	test_connect(udp_clt_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));

	/* Receive the COMM_UP notifications and get the associd's */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(udp_svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;

	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_clt_loop, sizeof(udp_clt_loop));
	paddrparams.spp_hbinterval = 1000;
	paddrparams.spp_pathmaxrxt = dflt_pathmaxrxt+1;
	test_setsockopt(udp_svr_sk, SCTP_PEER_ADDR_PARAMS, &paddrparams,
							sizeof(paddrparams));
	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) - "
		 "one-to-many style valid associd valid address");

	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_svr_loop, sizeof(udp_svr_loop));
	paddrparams.spp_hbinterval = 1000;
	paddrparams.spp_pathmaxrxt = dflt_pathmaxrxt+1;
	
        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid transport error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid transport");

	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_clt_loop, sizeof(udp_clt_loop));
	paddrparams.spp_hbinterval = 1000;
	paddrparams.spp_pathmaxrxt = dflt_pathmaxrxt+1;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams) - 1);
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid parameter length error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid parameter length");

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_DELAYED_ACK_TIME,
			   &value,
			   sizeof(value) - 1);
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_DELAYED_ACK_TIME) "
			 "invalid parameter length error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_DELAYED_ACK_TIME) "
		 "- one-to-many style invalid parameter length");

	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_clt_loop, sizeof(udp_clt_loop));
	paddrparams.spp_sackdelay = 501;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid sack delay error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid sack delay");

	value.assoc_id    = sac->sac_assoc_id;
	value.assoc_value = 501;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_DELAYED_ACK_TIME,
			   &value,
			   sizeof(value));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_DELAYED_ACK_TIME) "
			 "invalid sack delay error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_DELAYED_ACK_TIME) "
		 "- one-to-many style invalid sack delay");

	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_clt_loop, sizeof(udp_clt_loop));
	paddrparams.spp_pathmtu = 511;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid path MTU error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid path MTU");

	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_clt_loop, sizeof(udp_clt_loop));
	paddrparams.spp_flags = SPP_HB_ENABLE | SPP_HB_DISABLE;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid hb enable flags error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid hb enable flags");

	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_clt_loop, sizeof(udp_clt_loop));
	paddrparams.spp_flags = SPP_PMTUD_ENABLE | SPP_PMTUD_DISABLE;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid PMTU discovery enable flags error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid PMTU discovery enable flags");

	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_assoc_id = sac->sac_assoc_id;
	memcpy(&paddrparams.spp_address, &udp_clt_loop, sizeof(udp_clt_loop));
	paddrparams.spp_flags = SPP_SACKDELAY_ENABLE | SPP_SACKDELAY_DISABLE;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid sack delay enable flags error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid sack delay enable flags");

	memset(&paddrparams, 0, sizeof(paddrparams));
	paddrparams.spp_flags = SPP_HB_DEMAND;

        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			   &paddrparams,
			   sizeof(paddrparams));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
			 "invalid hb demand error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_PEER_ADDR_PARAMS) "
		 "- one-to-many style invalid hb demand");

	close(udp_svr_sk);
	close(udp_clt_sk);


	/* TEST #6: SCTP_DEFAULT_SEND_PARAM socket option. */
	/* Create and bind 2 UDP-style sockets(udp_svr_sk, udp_clt_sk) and
	 * 2 TCP-style sockets. (tcp_svr_sk, tcp_clt_sk)
	 */
	udp_svr_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
	udp_clt_sk = test_socket(pf_class, SOCK_SEQPACKET, IPPROTO_SCTP);
	tcp_svr_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);
	tcp_clt_sk = test_socket(pf_class, SOCK_STREAM, IPPROTO_SCTP);

	/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
	test_enable_assoc_change(udp_svr_sk);
	test_enable_assoc_change(udp_clt_sk);
	test_enable_assoc_change(tcp_svr_sk);
	test_enable_assoc_change(tcp_clt_sk);

	test_bind(udp_svr_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));
	test_bind(udp_clt_sk, &udp_clt_loop.sa, sizeof(udp_clt_loop));
	test_bind(tcp_svr_sk, &tcp_svr_loop.sa, sizeof(tcp_svr_loop));
	test_bind(tcp_clt_sk, &tcp_clt_loop.sa, sizeof(tcp_clt_loop));

	/* Mark udp_svr_sk and tcp_svr_sk as being able to accept new
	 * associations.
	 */
	test_listen(udp_svr_sk, 5);
	test_listen(tcp_svr_sk, 5);

	/* Set default send parameters on the unconnected UDP-style sockets. */
	memset(&set_udp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_udp_sk_dflt_param.sinfo_ppid = 1000;
	test_setsockopt(udp_svr_sk, SCTP_DEFAULT_SEND_PARAM,
			&set_udp_sk_dflt_param, sizeof(set_udp_sk_dflt_param));
	memset(&set_udp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_udp_sk_dflt_param.sinfo_ppid = 1000;
	test_setsockopt(udp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
			&set_udp_sk_dflt_param, sizeof(set_udp_sk_dflt_param));

	tst_resm(TPASS, "setsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-many style socket");

	/* Get default send parameters on the unconnected UDP-style socket. */
	memset(&get_udp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_udp_sk_dflt_param);
	test_getsockopt(udp_svr_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_udp_sk_dflt_param, &optlen);

	/* Verify that the get param matches set param. */
	if (set_udp_sk_dflt_param.sinfo_ppid !=
			get_udp_sk_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	/* Get default send parameters on the unconnected UDP-style socket. */
	memset(&get_udp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_udp_sk_dflt_param);
	test_getsockopt(udp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
		       &get_udp_sk_dflt_param, &optlen);

	/* Verify that the get param matches set param. */
	if (set_udp_sk_dflt_param.sinfo_ppid !=
			get_udp_sk_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	tst_resm(TPASS, "getsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-many style socket");

	/* Verify that trying to set send params with an invalid assoc id
	 * on an UDP-style socket fails.
	 */
	memset(&set_udp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_udp_sk_dflt_param.sinfo_ppid = 1000;
       	/* Invalid assoc id */
	set_udp_sk_dflt_param.sinfo_assoc_id = 1234;
        error = setsockopt(udp_clt_sk, SOL_SCTP, SCTP_DEFAULT_SEND_PARAM,
			   &set_udp_sk_dflt_param,
			   sizeof(set_udp_sk_dflt_param));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "invalid associd error:%d, errno:%d\n",
			 error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_DEFAULT_SEND_PARAM) "
		 "- one-to-many style invalid associd");

	/* Do a connect on a UDP-style socket and establish an association. */
	test_connect(udp_clt_sk, &udp_svr_loop.sa, sizeof(udp_svr_loop));

	/* Receive the COMM_UP notifications and get the associd's */
	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(udp_svr_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	udp_svr_associd = sac->sac_assoc_id;

	inmessage.msg_controllen = sizeof(incmsg);
	error = test_recvmsg(udp_clt_sk, &inmessage, MSG_WAITALL);
	test_check_msg_notification(&inmessage, error,
				    sizeof(struct sctp_assoc_change),
				    SCTP_ASSOC_CHANGE, SCTP_COMM_UP);	
	sac = (struct sctp_assoc_change *)iov.iov_base;
	udp_clt_associd = sac->sac_assoc_id;

	/* Verify that trying to set send params with an assoc id not 
	 * belonging to the socket on an UDP-style socket fails.
	 */
	memset(&set_udp_assoc_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_udp_assoc_dflt_param.sinfo_ppid = 3000;
	set_udp_assoc_dflt_param.sinfo_assoc_id = udp_clt_associd;
	error = setsockopt(udp_svr_sk, SOL_SCTP, SCTP_DEFAULT_SEND_PARAM,
			   &set_udp_assoc_dflt_param,
			   sizeof(set_udp_assoc_dflt_param));
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "setsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "associd belonging to another socket "
			 "error:%d, errno:%d", error, errno);

	tst_resm(TPASS, "setsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-many style associd belonging to another socket");

	/* Set default send parameters of an association on the listening 
	 * UDP-style socket with a valid associd.
	 */ 
	memset(&set_udp_assoc_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_udp_assoc_dflt_param.sinfo_ppid = 3000;
	set_udp_assoc_dflt_param.sinfo_assoc_id = udp_svr_associd;
	test_setsockopt(udp_svr_sk, SCTP_DEFAULT_SEND_PARAM,
			&set_udp_assoc_dflt_param,
			sizeof(set_udp_assoc_dflt_param));

	tst_resm(TPASS, "setsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-many style valid associd");

	/* Get default send parameters of an association on the listening 
	 * UDP-style socket with a valid associd.
	 */ 
	memset(&get_udp_assoc_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	get_udp_assoc_dflt_param.sinfo_assoc_id = udp_svr_associd ;
	optlen = sizeof(get_udp_assoc_dflt_param);
	test_getsockopt(udp_svr_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_udp_assoc_dflt_param, &optlen);

	/* Verify that the get param matches the set param. */
	if (get_udp_assoc_dflt_param.sinfo_ppid !=
			set_udp_assoc_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	tst_resm(TPASS, "getsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-many style valid associd");

	/* Get default send parameters of an association on the connected 
	 * UDP-style socket with zero associd. This should return the
	 * socket wide default parameters.
	 */ 
	memset(&get_udp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	get_udp_sk_dflt_param.sinfo_assoc_id = 0 ;
	optlen = sizeof(get_udp_sk_dflt_param);
	test_getsockopt(udp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_udp_sk_dflt_param, &optlen);

	/* Verify that the get param matches the socket-wide set param. */
	if (get_udp_sk_dflt_param.sinfo_ppid !=
			set_udp_sk_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	tst_resm(TPASS, "getsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-many style zero associd");

	peeloff_sk = test_sctp_peeloff(udp_svr_sk, udp_svr_associd); 

	/* Get default send parameters of an association on the peeled off 
	 * UDP-style socket. This should return the association's default
	 * parameters.
	 */ 
	memset(&get_peeloff_assoc_dflt_param, 0,
	       sizeof(struct sctp_sndrcvinfo));
	get_peeloff_assoc_dflt_param.sinfo_assoc_id = 0 ;
	optlen = sizeof(get_peeloff_assoc_dflt_param);
	test_getsockopt(peeloff_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_peeloff_assoc_dflt_param, &optlen);

	/* Verify that the get param matches the association's set param. */
	if (get_peeloff_assoc_dflt_param.sinfo_ppid !=
			set_udp_assoc_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	tst_resm(TPASS, "getsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-many style peeled off socket");

	/* Set default send parameters on the unconnected TCP-style sockets. */
	memset(&set_tcp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_tcp_sk_dflt_param.sinfo_ppid = 2000;
	/* Invalid assoc id, ignored on a TCP-style socket. */
	set_tcp_sk_dflt_param.sinfo_assoc_id = 1234;
	test_setsockopt(tcp_svr_sk, SCTP_DEFAULT_SEND_PARAM,
			&set_tcp_sk_dflt_param,
			sizeof(set_tcp_sk_dflt_param));

	/* Set default send parameters on the unconnected TCP-style sockets. */
	memset(&set_tcp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_tcp_sk_dflt_param.sinfo_ppid = 2000;
	/* Invalid assoc id, ignored on a TCP-style socket. */
	set_tcp_sk_dflt_param.sinfo_assoc_id = 1234;
	test_setsockopt(tcp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
			&set_tcp_sk_dflt_param,
			sizeof(set_tcp_sk_dflt_param));

	tst_resm(TPASS, "setsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-one style socket");

	/* Get default send parameters on the unconnected TCP-style socket. */
	memset(&get_tcp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_tcp_sk_dflt_param);
	test_getsockopt(tcp_svr_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_tcp_sk_dflt_param, &optlen);

	/* Verify that the get param matches set param. */
	if (set_tcp_sk_dflt_param.sinfo_ppid !=
			get_tcp_sk_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	/* Get default send parameters on the unconnected TCP-style socket. */
	memset(&get_tcp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_tcp_sk_dflt_param);
	test_getsockopt(tcp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_tcp_sk_dflt_param, &optlen);

	/* Verify that the get param matches set param. */
	if (set_tcp_sk_dflt_param.sinfo_ppid !=
			get_tcp_sk_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	tst_resm(TPASS, "getsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-one style socket");

	/* Do a connect on a TCP-style socket and establish an association. */
	test_connect(tcp_clt_sk, &tcp_svr_loop.sa, sizeof(tcp_svr_loop));

	/* Set default send parameters of an association on the connected 
	 * TCP-style socket.
	 */ 
	memset(&set_tcp_assoc_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	set_tcp_assoc_dflt_param.sinfo_ppid = 4000;
	set_tcp_assoc_dflt_param.sinfo_assoc_id = 0;
	test_setsockopt(tcp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
			&set_tcp_assoc_dflt_param,
			sizeof(set_tcp_assoc_dflt_param));

	tst_resm(TPASS, "setsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-one style assoc");

	/* Get default send parameters of an association on the connected 
	 * TCP-style socket.
	 */ 
	memset(&get_tcp_assoc_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_tcp_assoc_dflt_param);
	test_getsockopt(tcp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_tcp_assoc_dflt_param, &optlen);

	if (set_tcp_assoc_dflt_param.sinfo_ppid !=
			get_tcp_assoc_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	/* Get default send parameters on the connected TCP-style socket.  */ 
	memset(&get_tcp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_tcp_sk_dflt_param);
	test_getsockopt(tcp_clt_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_tcp_sk_dflt_param, &optlen);

	/* Verify that the get parameters returned matches the set param
	 * set for the association, not the socket-wide param.
	 */ 
	if ((get_tcp_sk_dflt_param.sinfo_ppid ==
			set_tcp_sk_dflt_param.sinfo_ppid) ||
	    (get_tcp_sk_dflt_param.sinfo_ppid !=
	    		set_tcp_assoc_dflt_param.sinfo_ppid))
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	/* Get default send parameters on the listening TCP-style socket.  */ 
	memset(&get_tcp_sk_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_tcp_sk_dflt_param);
	test_getsockopt(tcp_svr_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_tcp_sk_dflt_param, &optlen);

	/* Verify that the get parameters returned matches the socket-wide 
	 * set param.
	 */
	if (get_tcp_sk_dflt_param.sinfo_ppid !=
			set_tcp_sk_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	tst_resm(TPASS, "getsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-one style assoc");

	accept_sk = test_accept(tcp_svr_sk, NULL, &addrlen); 

	/* Get default send parameters of an association on the accepted 
	 * TCP-style socket.
	 */ 
	memset(&get_accept_assoc_dflt_param, 0, sizeof(struct sctp_sndrcvinfo));
	optlen = sizeof(get_accept_assoc_dflt_param);
	test_getsockopt(accept_sk, SCTP_DEFAULT_SEND_PARAM,
			&get_accept_assoc_dflt_param, &optlen);

	error = 0;

	/* Verify that the get parameters returned matches the socket-wide 
	 * set param.
	 */
	if (get_tcp_sk_dflt_param.sinfo_ppid !=
			set_tcp_sk_dflt_param.sinfo_ppid)
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_DEFAULT_SEND_PARAM) "
			 "mismatch.");

	tst_resm(TPASS, "getsockopt(SCTP_DEFAULT_SEND_PARAM) - "
		 "one-to-one style accepted socket");

	/* TEST #7: SCTP_GET_PEER_ADDR_INFO socket option. */
	/* Try 0 associd and 0 addr */
	memset(&pinfo, 0, sizeof(pinfo));
	optlen = sizeof(pinfo);
	error = getsockopt(udp_clt_sk, SOL_SCTP, SCTP_GET_PEER_ADDR_INFO,
			   &pinfo, &optlen);			   
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_GET_PEER_ADDR_INFO) "
			 "null associd, null addr error:%d, errno:%d\n",
			error, errno);

	tst_resm(TPASS, "getsockopt(SCTP_GET_PEER_ADDR_INFO) - "
		 "null associd and null addr");

	/* Try valid associd, but 0 addr */
	memset(&pinfo, 0, sizeof(pinfo));
	optlen = sizeof(pinfo);
	pinfo.spinfo_assoc_id = udp_clt_associd;
	error = getsockopt(udp_clt_sk, SOL_SCTP, SCTP_GET_PEER_ADDR_INFO,
			   &pinfo, &optlen);			   
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_GET_PEER_ADDR_INFO) "
			 "valid associd, null addr error:%d, errno:%d\n",
			error, errno);

	tst_resm(TPASS, "getsockopt(SCTP_GET_PEER_ADDR_INFO) - "
		 "valid associd and null addr");

	/* Try valid associd, invalid addr */
	memset(&pinfo, 0, sizeof(pinfo));
	optlen = sizeof(pinfo);
	pinfo.spinfo_assoc_id = udp_clt_associd;
	memcpy(&pinfo.spinfo_address, &udp_clt_loop, sizeof(udp_clt_loop));
	error = getsockopt(udp_clt_sk, SOL_SCTP, SCTP_GET_PEER_ADDR_INFO,
			   &pinfo, &optlen);			   
	if ((-1 != error) || (EINVAL != errno))
		tst_brkm(TBROK, tst_exit, "getsockopt(SCTP_GET_PEER_ADDR_INFO) "
			 "valid associd, invalid addr error:%d, errno:%d\n",
			error, errno);

	tst_resm(TPASS, "getsockopt(SCTP_GET_PEER_ADDR_INFO) - "
		 "valid associd and invalid addr");

	/* Try valid associd, valid addr */
	memset(&pinfo, 0, sizeof(pinfo));
	optlen = sizeof(pinfo);
	pinfo.spinfo_assoc_id = udp_clt_associd;
	memcpy(&pinfo.spinfo_address, &udp_svr_loop, sizeof(udp_svr_loop));
	test_getsockopt(udp_clt_sk, SCTP_GET_PEER_ADDR_INFO, &pinfo, &optlen);			   

	tst_resm(TPASS, "getsockopt(SCTP_GET_PEER_ADDR_INFO) - "
		 "valid associd and valid addr");

	/* Try valid addr, peeled off socket */
	memset(&pinfo, 0, sizeof(pinfo));
	optlen = sizeof(pinfo);
	pinfo.spinfo_assoc_id = 0;
	memcpy(&pinfo.spinfo_address, &udp_clt_loop, sizeof(udp_clt_loop));
	test_getsockopt(peeloff_sk, SCTP_GET_PEER_ADDR_INFO, &pinfo, &optlen);			   

	tst_resm(TPASS, "getsockopt(SCTP_GET_PEER_ADDR_INFO) - "
		 "valid associd and valid addr peeled off socket");

	/* Try valid addr, TCP-style accept socket */
	memset(&pinfo, 0, sizeof(pinfo));
	optlen = sizeof(pinfo);
	pinfo.spinfo_assoc_id = 0;
	memcpy(&pinfo.spinfo_address, &tcp_clt_loop, sizeof(tcp_clt_loop));
	error = test_getsockopt(accept_sk, SCTP_GET_PEER_ADDR_INFO, &pinfo,
				&optlen);			   

	tst_resm(TPASS, "getsockopt(SCTP_GET_PEER_ADDR_INFO) - "
		 "valid associd and valid addr accepted socket");

	close(udp_svr_sk);
	close(udp_clt_sk);
	close(tcp_svr_sk);
	close(tcp_clt_sk);
	close(accept_sk);
	close(peeloff_sk);

        /* Indicate successful completion.  */
        return 0;
}
