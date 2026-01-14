// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef TSO_SAFE_NET_H__
#define TSO_SAFE_NET_H__

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

#endif /* TSO_SAFE_NET_H__ */
