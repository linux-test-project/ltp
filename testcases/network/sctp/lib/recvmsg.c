/* SCTP kernel reference Implementation: User API extensions.
 *
 * sctp_recvmsg.c
 *
 * Distributed under the terms of the LGPL v2.1 as described in
 *    http://www.gnu.org/copyleft/lesser.txt
 *
 * This file is part of the user library that offers support for the
 * SCTP kernel reference Implementation. The main purpose of this
 * code is to provide the SCTP Socket API mappings for user
 * application to interface with the SCTP in kernel.
 *
 * This implementation is based on the Socket API Extensions for SCTP
 * defined in <draft-ietf-tsvwg-sctpsocket-10.txt>
 *
 * Copyright (c) 2003 International Business Machines, Corp.
 *
 * Written or modified by:
 *  Ryan Layer	<rmlayer@us.ibm.com>
 *
 * An implementation may provide a library function (or possibly system
 * call) to assist the user with the advanced features of SCTP. Note
 * that in order for the sctp_sndrcvinfo structure to be filled in by
 * sctp_recvmsg() the caller must enable the sctp_data_io_events with
 * the SCTP_EVENTS option.
 *
 * sctp_recvmsg(). Its syntax is,
 *
 * int sctp_recvmsg(int s,
 *		    void *msg,
 *		    size_t len,
 *		    struct sockaddr *from,
 *		    socklen_t *fromlen,
 *		    struct sctp_sndrcvinfo *sinfo,
 *		    int *msg_flags)
 *
 *
 * s          - is the socket descriptor
 * msg        - is a message buffer to be filled.
 * len        - is the length of the message buffer.
 * from       - is a pointer to a address to be filled with
 *		the sender of this messages address.
 * fromlen    - is the from length.
 * sinfo      - A pointer to a sctp_sndrcvinfo structure
 *		to be filled upon receipt of the message.
 * msg_flags  - A pointer to a integer to be filled with
 *		any message flags (e.g. MSG_NOTIFICATION).
 */

#include <string.h>
#include <errno.h>
#include <sys/socket.h>   /* struct sockaddr_storage, setsockopt() */
#include <netinet/sctp.h>

int sctp_recvmsg(int s, void *msg, size_t len, struct sockaddr *from,
		 socklen_t *fromlen, struct sctp_sndrcvinfo *sinfo,
		 int *msg_flags)
{
	int error;
	struct iovec iov;
	struct msghdr inmsg;
	char incmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
	struct cmsghdr *cmsg = NULL;

	memset(&inmsg, 0, sizeof (inmsg));

	iov.iov_base = msg;
	iov.iov_len = len;

	inmsg.msg_name = from;
	inmsg.msg_namelen = fromlen ? *fromlen : 0;
	inmsg.msg_iov = &iov;
	inmsg.msg_iovlen = 1;
	inmsg.msg_control = incmsg;
	inmsg.msg_controllen = sizeof(incmsg);

	error = recvmsg(s, &inmsg, 0);
	if (error < 0)
		return error;

	if (fromlen)
		*fromlen = inmsg.msg_namelen;
	if (msg_flags)
		*msg_flags = inmsg.msg_flags;

	if (!sinfo)
		return error;

	for (cmsg = CMSG_FIRSTHDR(&inmsg); cmsg != NULL;
				 cmsg = CMSG_NXTHDR(&inmsg, cmsg)) {
		if ((IPPROTO_SCTP == cmsg->cmsg_level) &&
		    (SCTP_SNDRCV == cmsg->cmsg_type))
			break;
	}

        /* Copy sinfo. */
	if (cmsg)
		memcpy(sinfo, CMSG_DATA(cmsg), sizeof(struct sctp_sndrcvinfo));

	return (error);
}
