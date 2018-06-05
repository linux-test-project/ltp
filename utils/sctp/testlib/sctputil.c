/* SCTP kernel Implementation
 * (C) Copyright IBM Corp. 2001, 2003
 * Copyright (C) 1999 Cisco
 * Copyright (C) 1999-2000 Motorola
 # Copyright (C) 2001 Nokia
 * Copyright (C) 2001 La Monte H.P. Yarroll
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
 *    Narasimha Budihal <narsi@refcode.org>
 *    Karl Knutson <karl@athena.chicago.il.us>
 *    Jon Grimm <jgrimm@us.ibm.com>
 *    Daisy Chang <daisyc@us.ibm.com>
 *    Sridhar Samudrala <sri@us.ibm.com>
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <errno.h>
#include <malloc.h>
#include "netinet/sctp.h"
#include "sctputil.h"

/* This function prints the cmsg data. */
void
test_print_cmsg(sctp_cmsg_t type, sctp_cmsg_data_t *data)
{
	switch(type) {
	case SCTP_INIT:
		printf("INIT\n");
		printf("   sinit_num_ostreams %d\n",
		       data->init.sinit_num_ostreams);
		printf("   sinit_max_instreams %d\n",
		       data->init.sinit_max_instreams);
		printf("   sinit_max_attempts %d\n",
		       data->init.sinit_max_attempts);
		printf("   sinit_max_init_timeo %d\n",
		       data->init.sinit_max_init_timeo);

		break;
	case SCTP_SNDRCV:
		printf("SNDRCV\n");
		printf("   sinfo_stream %u\n",	data->sndrcv.sinfo_stream);
		printf("   sinfo_ssn %u\n",	data->sndrcv.sinfo_ssn);
		printf("   sinfo_flags 0x%x\n",	data->sndrcv.sinfo_flags);
		printf("   sinfo_ppid %u\n",	data->sndrcv.sinfo_ppid);
		printf("   sinfo_context %x\n",	data->sndrcv.sinfo_context);
		printf("   sinfo_tsn     %u\n",    data->sndrcv.sinfo_tsn);
		printf("   sinfo_cumtsn  %u\n",    data->sndrcv.sinfo_cumtsn);
		printf("   sinfo_assoc_id  %u\n", data->sndrcv.sinfo_assoc_id);

		break;

	default:
		printf("UNKNOWN CMSG: %d\n", type);
		break;
	}
}

/* This function prints the message. */
void
test_print_message(int sk, struct msghdr *msg, size_t msg_len)
{
	sctp_cmsg_data_t *data;
	struct cmsghdr *cmsg;
	int i;
	int done = 0;
	char save;
	union sctp_notification *sn;

	for (cmsg = CMSG_FIRSTHDR(msg);
	     cmsg != NULL;
	     cmsg = CMSG_NXTHDR(msg, cmsg)) {
		     data = (sctp_cmsg_data_t *)CMSG_DATA(cmsg);
		     test_print_cmsg(cmsg->cmsg_type, data);
	}

	if (!(MSG_NOTIFICATION & msg->msg_flags)) {
		int index = 0;
		/* Make sure that everything is printable and that we
		 * are NUL terminated...
		 */
		printf("DATA(%d):  ", msg_len);
		while ( msg_len > 0 ) {
			char *text;
			int len;

			text = msg->msg_iov[index].iov_base;
			len = msg->msg_iov[index].iov_len;

                        save = text[msg_len-1];
			if ( len > msg_len ) {
                                text[(len = msg_len) - 1] = '\0';
                        }

			if ( (msg_len -= len) > 0 ) { index++; }

			for (i = 0; i < len - 1; ++i) {
                                if (!isprint(text[i])) text[i] = '.';
                        }

			printf("%s", text);
			text[msg_len-1] = save;

			if ( (done = !strcmp(text, "exit")) ) { break; }
		}
	} else {
		printf("NOTIFICATION: ");
		sn = (union sctp_notification *)msg->msg_iov[0].iov_base;
		switch (sn->sn_header.sn_type) {
		case SCTP_ASSOC_CHANGE:
			switch (sn->sn_assoc_change.sac_state) {
			case SCTP_COMM_UP:
				printf("ASSOC_CHANGE - COMM_UP");
				break;
			case SCTP_COMM_LOST:
				printf("ASSOC_CHANGE - COMM_LOST");
				break;
			case SCTP_RESTART:
				printf("ASSOC_CHANGE - RESTART");
				break;
			case SCTP_SHUTDOWN_COMP:
				printf("ASSOC_CHANGE - SHUTDOWN_COMP");
				break;
			case SCTP_CANT_STR_ASSOC:
				printf("ASSOC_CHANGE - CANT_STR_ASSOC");
				break;
			default:
				printf("ASSOC_CHANGE - UNEXPECTED(%d)",
				       sn->sn_assoc_change.sac_state);
				break;
			}
			break;
		default:
			printf("%d", sn->sn_header.sn_type);
			break;
		}
	}

	printf("\n");
}

/* Check if a buf/msg_flags matches a notification, its type, and possibly an
 * additional field in the corresponding notification structure.
 */
void
test_check_buf_notification(void *buf, int datalen, int msg_flags,
			    int expected_datalen, uint16_t expected_sn_type,
			    uint32_t expected_additional)
{
	union sctp_notification *sn;

	if (!(msg_flags & MSG_NOTIFICATION))
		tst_brkm(TBROK, tst_exit,
			 "Got a datamsg, expecting notification");

	if (expected_datalen <= 0)
		return;

	if (datalen != expected_datalen)
		tst_brkm(TBROK, tst_exit,
			 "Got a notification of unexpected length:%d, expected length:%d",
			  datalen, expected_datalen);

	sn = (union sctp_notification *)buf;
	if (sn->sn_header.sn_type != expected_sn_type)
		tst_brkm(TBROK, tst_exit,
			 "Unexpected notification:%d expected:%d",
			  sn->sn_header.sn_type, expected_sn_type);

	switch(sn->sn_header.sn_type){
	case SCTP_ASSOC_CHANGE:
		if (sn->sn_assoc_change.sac_state != expected_additional)
			tst_brkm(TBROK, tst_exit,
				 "Unexpected sac_state:%d expected:%d",
				  sn->sn_assoc_change.sac_state, expected_additional);
		break;
	default:
		break;
	}
}

/* Check if a message matches a notification, its type, and possibly an
 * additional field in the corresponding notification structure.
 */
void
test_check_msg_notification(struct msghdr *msg, int datalen,
			    int expected_datalen, uint16_t expected_sn_type,
			    uint32_t expected_additional)
{
	test_check_buf_notification(msg->msg_iov[0].iov_base, datalen,
				    msg->msg_flags, expected_datalen,
				    expected_sn_type, expected_additional);
}

/* Check if a buf/msg_flags/sinfo corresponds to data, its length, msg_flags,
 * stream and ppid.
 */
void
test_check_buf_data(void *buf, int datalen, int msg_flags,
		    struct sctp_sndrcvinfo *sinfo, int expected_datalen,
		    int expected_msg_flags, uint16_t expected_stream,
		    uint32_t expected_ppid)
{
	if (msg_flags & MSG_NOTIFICATION)
		tst_brkm(TBROK, tst_exit,
			 "Got a notification, expecting a datamsg");

	if (expected_datalen <= 0)
		return;

	if (datalen != expected_datalen)
		tst_brkm(TBROK, tst_exit,
			 "Got a datamsg of unexpected length:%d, expected length:%d",
			  datalen, expected_datalen);

	if ((msg_flags & ~0x80000000) != expected_msg_flags)
		tst_brkm(TBROK, tst_exit,
			 "Unexpected msg_flags:0x%x expecting:0x%x",
			  msg_flags, expected_msg_flags);

	if ((0 == expected_stream) && (0 == expected_ppid))
		return;

	if (!sinfo)
		tst_brkm(TBROK, tst_exit,
			 "Null sinfo, but expected stream:%d expected ppid:%d",
			  expected_stream, expected_ppid);

	if (sinfo->sinfo_stream != expected_stream)
		tst_brkm(TBROK, tst_exit,
			 "stream mismatch: expected:%x got:%x",
			  expected_stream, sinfo->sinfo_stream);
	if (sinfo->sinfo_ppid != expected_ppid)
		tst_brkm(TBROK, tst_exit,
			 "ppid mismatch: expected:%x got:%x\n",
			  expected_ppid, sinfo->sinfo_ppid);
}

/* Check if a message corresponds to data, its length, msg_flags, stream and
 * ppid.
 */
void
test_check_msg_data(struct msghdr *msg, int datalen, int expected_datalen,
		    int expected_msg_flags, uint16_t expected_stream,
		    uint32_t expected_ppid)
{
	struct cmsghdr *cmsg = NULL;
	struct sctp_sndrcvinfo *sinfo = NULL;

	/* Receive auxiliary data in msgh. */
	for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
				 cmsg = CMSG_NXTHDR(msg, cmsg)){
		if (IPPROTO_SCTP == cmsg->cmsg_level &&
		    SCTP_SNDRCV == cmsg->cmsg_type)
			break;
	} /* for( all cmsgs) */

	if ((!cmsg) ||
	    (cmsg->cmsg_len < CMSG_LEN(sizeof(struct sctp_sndrcvinfo))))
		sinfo = NULL;
	else
		sinfo = (struct sctp_sndrcvinfo *)CMSG_DATA(cmsg);

	test_check_buf_data(msg->msg_iov[0].iov_base, datalen, msg->msg_flags,
			    sinfo, expected_datalen, expected_msg_flags,
			    expected_stream, expected_ppid);

}


/* Allocate a buffer of requested len and fill in with data. */
void *
test_build_msg(int len)
{
	int i = len - 1;
	int n;
	unsigned char msg[] =
		"012345678901234567890123456789012345678901234567890";
	char *msg_buf, *p;

	msg_buf = (char *)malloc(len);
	if (!msg_buf)
		tst_brkm(TBROK, tst_exit, "malloc failed");

	p = msg_buf;

	do {
		n = ((i > 50)?50:i);
		memcpy(p, msg, ((i > 50)?50:i));
		p += n;
		i -= n;
	} while (i > 0);

	msg_buf[len-1] = '\0';

	return(msg_buf);
}

/* Enable ASSOC_CHANGE and SNDRCVINFO notifications. */
void test_enable_assoc_change(int fd)
{
	struct sctp_event_subscribe subscribe;

	memset(&subscribe, 0, sizeof(subscribe));
	subscribe.sctp_data_io_event = 1;
	subscribe.sctp_association_event = 1;
	test_setsockopt(fd, SCTP_EVENTS, (char *)&subscribe,
		        sizeof(subscribe));
}

static int cmp_addr(sockaddr_storage_t *addr1, sockaddr_storage_t *addr2)
{
	if (addr1->sa.sa_family != addr2->sa.sa_family)
		return 0;
	switch (addr1->sa.sa_family) {
	case AF_INET6:
		if (addr1->v6.sin6_port != addr2->v6.sin6_port)
			return -1;
		return memcmp(&addr1->v6.sin6_addr, &addr2->v6.sin6_addr,
			      sizeof(addr1->v6.sin6_addr));
	case AF_INET:
		if (addr1->v4.sin_port != addr2->v4.sin_port)
			return 0;
		return memcmp(&addr1->v4.sin_addr, &addr2->v4.sin_addr,
			      sizeof(addr1->v4.sin_addr));
	default:
		tst_brkm(TBROK, tst_exit,
			 "invalid address type %d", addr1->sa.sa_family);
		return -1;
	}
}

/* Test peer addresses for association. */
int test_peer_addr(int sk, sctp_assoc_t asoc, sockaddr_storage_t *peers, int count)
{
	struct sockaddr *addrs;
	int error, i, j;
	struct sockaddr *sa_addr;
	socklen_t addrs_size = 0;
	void *addrbuf;
	char found[count];
	memset(found, 0, count);

	error = sctp_getpaddrs(sk, asoc, &addrs);
	if (-1 == error) {
		tst_brkm(TBROK, tst_exit,
			  "sctp_getpaddrs: %s", strerror(errno));
		return error;
	}
	if (error != count) {
		sctp_freepaddrs(addrs);
		tst_brkm(TBROK, tst_exit,
			 "peer count %d mismatch, expected %d",
			  error, count);
	}
	addrbuf = addrs;
	for (i = 0; i < count; i++) {
		sa_addr = (struct sockaddr *)addrbuf;
		switch (sa_addr->sa_family) {
		case AF_INET:
			addrs_size += sizeof(struct sockaddr_in);
			addrbuf += sizeof(struct sockaddr_in);
			break;
		case AF_INET6:
			addrs_size += sizeof(struct sockaddr_in6);
			addrbuf += sizeof(struct sockaddr_in6);
			break;
		default:
			errno = EINVAL;
			sctp_freepaddrs(addrs);
			tst_brkm(TBROK, tst_exit,
				 "sctp_getpaddrs: %s", strerror(errno));
			return -1;
		}
		for (j = 0; j < count; j++) {
			if (cmp_addr((sockaddr_storage_t *)sa_addr,
				     &peers[j]) == 0) {
				found[j] = 1;
			}
		}
	}
	for (j = 0; j < count; j++) {
		if (found[j] == 0) {
			tst_brkm(TBROK, tst_exit,
				 "peer address %d not found", j);
		}
	}
	sctp_freepaddrs(addrs);
	return 0;
}
