/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TST_SAFE_NET_H__
#define TST_SAFE_NET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "safe_net_fn.h"

#define SAFE_SOCKET(domain, type, protocol) \
	safe_socket(__FILE__, __LINE__, NULL, domain, type, protocol)

#define SAFE_SETSOCKOPT(fd, level, optname, optval, optlen) \
	safe_setsockopt(__FILE__, __LINE__, fd, level, optname, optval, optlen)

#define SAFE_SEND(strict, sockfd, buf, len, flags) \
	safe_send(__FILE__, __LINE__, strict, sockfd, buf, len, flags)

#define SAFE_SENDTO(strict, fd, buf, len, flags, dest_addr, addrlen) \
	safe_sendto(__FILE__, __LINE__, strict, fd, buf, len, flags, \
		    dest_addr, addrlen)

#define SAFE_BIND(socket, address, address_len) \
	safe_bind(__FILE__, __LINE__, NULL, socket, address, \
		  address_len)

#define SAFE_LISTEN(socket, backlog) \
	safe_listen(__FILE__, __LINE__, NULL, socket, backlog)

#define SAFE_CONNECT(sockfd, addr, addrlen) \
	safe_connect(__FILE__, __LINE__, NULL, sockfd, addr, addrlen)

#define SAFE_GETSOCKNAME(sockfd, addr, addrlen) \
	safe_getsockname(__FILE__, __LINE__, NULL, sockfd, addr, \
			 addrlen)

#define SAFE_GETHOSTNAME(name, size) \
	safe_gethostname(__FILE__, __LINE__, name, size)

#endif /* TST_SAFE_NET_H__ */
