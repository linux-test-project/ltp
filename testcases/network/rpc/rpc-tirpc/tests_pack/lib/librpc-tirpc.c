/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int bound_socket(int domain, int type)
{
	int sock;
	struct sockaddr_storage addr;
	struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
	struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
	socklen_t slen;

	switch (domain) {
	case AF_INET:
		addr4->sin_family = AF_INET;
		addr4->sin_port = 0;
		addr4->sin_addr.s_addr = INADDR_ANY;
		slen = sizeof(*addr4);
		break;

	case AF_INET6:
		addr6->sin6_family = AF_INET6;
		addr6->sin6_port = 0;
		addr6->sin6_addr = in6addr_any;
		slen = sizeof(*addr6);
		break;

	default:
		errno = EAFNOSUPPORT;
		return -1;
	}

	if ((type != SOCK_STREAM) && (type != SOCK_DGRAM)) {
		errno = EINVAL;
		return -1;
	}

	sock = socket(domain, type, 0);
	if (sock < 0)
		return -1;

	if (bind(sock, (struct sockaddr *)&addr, slen) < 0) {
		close(sock);
		return -1;
	}

	return sock;
}
