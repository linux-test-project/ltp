/*
 * Copyright (c) 2015 Fujitsu Ltd.
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

#ifndef OLD_SAFE_NET_H__
#define OLD_SAFE_NET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "safe_net_fn.h"

#define SAFE_SOCKET(cleanup_fn, domain, type, protocol) \
	safe_socket(__FILE__, __LINE__, (cleanup_fn), domain, type, protocol)

#define SAFE_BIND(cleanup_fn, socket, address, address_len) \
	safe_bind(__FILE__, __LINE__, (cleanup_fn), socket, address, \
		  address_len)

#define SAFE_LISTEN(cleanup_fn, socket, backlog) \
	safe_listen(__FILE__, __LINE__, (cleanup_fn), socket, backlog)

#define SAFE_CONNECT(cleanup_fn, sockfd, addr, addrlen) \
	safe_connect(__FILE__, __LINE__, (cleanup_fn), sockfd, addr, addrlen)

#define SAFE_GETSOCKNAME(cleanup_fn, sockfd, addr, addrlen) \
	safe_getsockname(__FILE__, __LINE__, (cleanup_fn), sockfd, addr, \
			 addrlen)

#define TST_GET_UNUSED_PORT(cleanup_fn, family, type) \
	tst_get_unused_port(__FILE__, __LINE__, (cleanup_fn), family, type)

#endif /* OLD_SAFE_NET_H__ */
