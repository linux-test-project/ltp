/* SCTP kernel reference Implementation: User API extensions.
 *
 * connectx.c
 *
 * Distributed under the terms of the LGPL v2.1 as described in
 * http://www.gnu.org/copyleft/lesser.txt.
 *
 * This file is part of the user library that offers support for the
 * SCTP kernel reference Implementation. The main purpose of this
 * code is to provide the SCTP Socket API mappings for user
 * application to interface with the SCTP in kernel.
 *
 * This implementation is based on the Socket API Extensions for SCTP
 * defined in <draft-ietf-tsvwg-sctpsocket-10.txt.
 *
 * (C) Copyright IBM Corp. 2001, 2005
 *
 * Written or modified by:
 *   Frank Filz     <ffilz@us.ibm.com>
 */

#include <sys/socket.h>   /* struct sockaddr_storage, setsockopt() */
#include <netinet/in.h>
#include <netinet/sctp.h> /* SCTP_SOCKOPT_CONNECTX_* */
#include <errno.h>

/* Support the sctp_connectx() interface.
 *
 * See Sockets API Extensions for SCTP. Section 8.1.
 *
 * Instead of implementing through a socket call in sys_socketcall(),
 * tunnel the request through setsockopt().
 */
int
sctp_connectx(int fd, struct sockaddr *addrs, int addrcnt)
{
	void *addrbuf;
	struct sockaddr *sa_addr;
	socklen_t addrs_size = 0;
	int i;

	addrbuf = addrs;
	for (i = 0; i < addrcnt; i++) {
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
			return -1;
		}
	}

	return setsockopt(fd, SOL_SCTP, SCTP_SOCKOPT_CONNECTX, addrs, addrs_size);
}