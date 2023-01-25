/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_SAFE_NET_H__
#define TST_SAFE_NET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "safe_net_fn.h"
#include "tst_net.h"

#define SAFE_SOCKET(domain, type, protocol) \
	safe_socket(__FILE__, __LINE__, NULL, domain, type, protocol)

#define SAFE_SOCKETPAIR(domain, type, protocol, sv) \
	safe_socketpair(__FILE__, __LINE__, domain, type, protocol, sv)

#define SAFE_GETSOCKOPT(fd, level, optname, optval, optlen) \
	safe_getsockopt(__FILE__, __LINE__, fd, level, optname, optval, optlen)

#define SAFE_SETSOCKOPT(fd, level, optname, optval, optlen) \
	safe_setsockopt(__FILE__, __LINE__, fd, level, optname, optval, optlen)

#define SAFE_SETSOCKOPT_INT(fd, l, n, val) \
	do { \
		int v = val; \
		safe_setsockopt(__FILE__, __LINE__, fd, l, n, &v, sizeof(v)); \
	} while (0)

#define SAFE_SEND(strict, sockfd, buf, len, flags) \
	safe_send(__FILE__, __LINE__, strict, sockfd, buf, len, flags)

#define SAFE_SENDTO(strict, fd, buf, len, flags, dest_addr, addrlen) \
	safe_sendto(__FILE__, __LINE__, strict, fd, buf, len, flags, \
		    dest_addr, addrlen)

#define SAFE_SENDMSG(msg_len, fd, msg, flags) \
	safe_sendmsg(__FILE__, __LINE__, msg_len, fd, msg, flags)

#define SAFE_RECV(msg_len, fd, buf, size, flags)		\
	safe_recv(__FILE__, __LINE__, (msg_len), (fd), (buf), (size), (flags))

#define SAFE_RECVMSG(msg_len, fd, msg, flags)		\
	safe_recvmsg(__FILE__, __LINE__, msg_len, fd, msg, flags)

#define SAFE_BIND(socket, address, address_len) \
	safe_bind(__FILE__, __LINE__, NULL, socket, address, \
		  address_len)

#define SAFE_LISTEN(socket, backlog) \
	safe_listen(__FILE__, __LINE__, NULL, socket, backlog)

#define SAFE_ACCEPT(sockfd, addr, addrlen) \
	safe_accept(__FILE__, __LINE__, NULL, sockfd, addr, addrlen)

#define SAFE_CONNECT(sockfd, addr, addrlen) \
	safe_connect(__FILE__, __LINE__, NULL, sockfd, addr, addrlen)

#define SAFE_GETSOCKNAME(sockfd, addr, addrlen) \
	safe_getsockname(__FILE__, __LINE__, NULL, sockfd, addr, \
			 addrlen)

#define SAFE_GETHOSTNAME(name, size) \
	safe_gethostname(__FILE__, __LINE__, name, size)

#define SAFE_SETHOSTNAME(name, size) \
	safe_sethostname(__FILE__, __LINE__, name, size)

#define TST_GETSOCKPORT(sockfd) \
	tst_getsockport(__FILE__, __LINE__, sockfd)

#define TST_GET_UNUSED_PORT(family, type) \
	tst_get_unused_port(__FILE__, __LINE__, NULL, family, type)

/* new API only */

#define SAFE_GETADDRINFO(src_addr, port, hints, addr_info) \
	safe_getaddrinfo(__FILE__, __LINE__, src_addr, port, hints, addr_info)

#endif /* TST_SAFE_NET_H__ */
