/* SCTP kernel reference Implementation: User API extensions.
 *
 * addrs.c
 *
 * Distributed under the terms of the LGPL v2.1 as described in
 * ./COPYING.
 *
 * This file is part of the user library that offers support for the
 * SCTP kernel reference Implementation. The main purpose of this
 * code is to provide the SCTP Socket API mappings for user
 * application to interface with the SCTP in kernel.
 *
 * This implementation is based on the Socket API Extensions for SCTP
 * defined in <draft-ietf-tsvwg-sctpsocket-07.txt.
 *
 * (C) Copyright IBM Corp. 2003
 * Copyright (c) 2001-2002 Intel Corp.
 *
 * Written or modified by:
 *  Ardelle Fan     <ardelle.fan@intel.com>
 *  Sridhar Samudrala <sri@us.ibm.com>
 */

#include <malloc.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

/* Get all peer address on a socket.  This is a new SCTP API
 * described in the section 8.3 of the Sockets API Extensions for SCTP.
 * This is implemented using the getsockopt() interface.
 */
int
sctp_getpaddrs(int sd, sctp_assoc_t id, struct sockaddr **addrs)
{
	int len = sizeof(sctp_assoc_t);
	int cnt, err;
	struct sctp_getaddrs getaddrs;

	cnt = getsockopt(sd, SOL_SCTP, SCTP_GET_PEER_ADDRS_NUM, &id, &len);
	if (cnt < 0)
		return -1;

	if (0 == cnt) {
		*addrs = NULL;
		return 0;
	}

	len = cnt * sizeof(struct sockaddr_in6);

	getaddrs.assoc_id = id;
	getaddrs.addr_num = cnt;
	getaddrs.addrs = (struct sockaddr *)malloc(len);
	if (NULL == getaddrs.addrs)
		return -1;

	len = sizeof(getaddrs);
	err = getsockopt(sd, SOL_SCTP, SCTP_GET_PEER_ADDRS, &getaddrs, &len);
	if (err < 0)
		return -1;

	*addrs = getaddrs.addrs;

	return getaddrs.addr_num;

} /* sctp_getpaddrs() */

/* Frees all resources allocated by sctp_getpaddrs().  This is a new SCTP API
 * described in the section 8.4 of the Sockets API Extensions for SCTP.
 */
int
sctp_freepaddrs(struct sockaddr *addrs)
{
	free(addrs);
	return 0;

} /* sctp_freepaddrs() */

/* Get all locally bound address on a socket.  This is a new SCTP API
 * described in the section 8.5 of the Sockets API Extensions for SCTP.
 * This is implemented using the getsockopt() interface.
 */
int
sctp_getladdrs(int sd, sctp_assoc_t id, struct sockaddr **addrs)
{
	int len = sizeof(sctp_assoc_t);
	int cnt, err;
	struct sctp_getaddrs getaddrs;

	cnt = getsockopt(sd, SOL_SCTP, SCTP_GET_LOCAL_ADDRS_NUM, &id, &len);
	if (cnt < 0)
		return -1;

	if (0 == cnt) {
		*addrs = NULL;
		return 0;
	}

	len = cnt * sizeof(struct sockaddr_in6);

	getaddrs.assoc_id = id;
	getaddrs.addr_num = cnt;
	getaddrs.addrs = (struct sockaddr *)malloc(len);
	if (NULL == getaddrs.addrs)
		return -1;

	len = sizeof(getaddrs);
	err = getsockopt(sd, SOL_SCTP, SCTP_GET_LOCAL_ADDRS, &getaddrs, &len);
	if (err < 0)
		return -1;

	*addrs = getaddrs.addrs;

	return getaddrs.addr_num;

} /* sctp_getladdrs() */

/* Frees all resources allocated by sctp_getladdrs().  This is a new SCTP API
 * described in the section 8.6 of the Sockets API Extensions for SCTP.
 */
int
sctp_freeladdrs(struct sockaddr *addrs)
{
	free(addrs);
	return 0;

} /* sctp_freeladdrs() */
