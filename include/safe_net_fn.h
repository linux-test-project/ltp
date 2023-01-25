/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2015 Fujitsu Ltd.
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

#ifndef SAFE_NET_FN_H__
#define SAFE_NET_FN_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

int safe_socket(const char *file, const int lineno, void (cleanup_fn)(void),
		int domain, int type, int protocol);

int safe_socketpair(const char *file, const int lineno, int domain, int type,
		    int protocol, int sv[]);

int safe_getsockopt(const char *file, const int lineno, int sockfd, int level,
		    int optname, void *optval, socklen_t *optlen);

int safe_setsockopt(const char *file, const int lineno, int sockfd, int level,
		    int optname, const void *optval, socklen_t optlen);

ssize_t safe_send(const char *file, const int lineno, char len_strict,
		  int sockfd, const void *buf, size_t len, int flags);

ssize_t safe_sendto(const char *file, const int lineno, char len_strict,
		    int sockfd, const void *buf, size_t len, int flags,
		    const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t safe_sendmsg(const char *file, const int lineno, size_t msg_len,
		  int sockfd, const struct msghdr *msg, int flags);

ssize_t safe_recv(const char *file, const int lineno, size_t len,
	int sockfd, void *buf, size_t size, int flags);

ssize_t safe_recvmsg(const char *file, const int lineno, size_t msg_len,
		  int sockfd, struct msghdr *msg, int flags);

int safe_bind(const char *file, const int lineno, void (cleanup_fn)(void),
	      int socket, const struct sockaddr *address,
	      socklen_t address_len);

int safe_listen(const char *file, const int lineno, void (cleanup_fn)(void),
		int socket, int backlog);

int safe_accept(const char *file, const int lineno, void (cleanup_fn)(void),
		int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int safe_connect(const char *file, const int lineno, void (cleanup_fn)(void),
		 int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int safe_getsockname(const char *file, const int lineno,
		     void (cleanup_fn)(void), int sockfd, struct sockaddr *addr,
		     socklen_t *addrlen);

int safe_gethostname(const char *file, const int lineno,
		     char *name, size_t size);

int safe_sethostname(const char *file, const int lineno,
		     const char *name, size_t size);

int tst_getsockport(const char *file, const int lineno, int sockfd);

unsigned short tst_get_unused_port(const char *file, const int lineno,
	      void (cleanup_fn)(void), unsigned short family, int type);

char *tst_sock_addr(const struct sockaddr *sa, socklen_t salen, char *res,
		    size_t len);

#endif /* SAFE_NET_FN_H__ */
