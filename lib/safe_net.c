/*
 * Copyright (c) 2015 Fujitsu Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <errno.h>
#include "test.h"
#include "safe_macros_fn.h"
#include "safe_net_fn.h"

char *tst_sock_addr(const struct sockaddr *sa, socklen_t salen, char *res,
		    size_t len)
{
	char portstr[8];

	switch (sa->sa_family) {

	case AF_INET: {
		struct sockaddr_in *sin = (struct sockaddr_in *)sa;

		if (!inet_ntop(AF_INET, &sin->sin_addr, res, len))
			return NULL;

		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d",
				 ntohs(sin->sin_port));
			strcat(res, portstr);
		}

		return res;
	}

	case AF_INET6: {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;

		res[0] = '[';
		if (!inet_ntop(AF_INET6, &sin6->sin6_addr, res + 1, len - 1))
			return NULL;

		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), "]:%d",
				 ntohs(sin6->sin6_port));
			strcat(res, portstr);
			return res;
		}

		return res + 1;
	}

	case AF_UNIX: {
		struct sockaddr_un *unp = (struct sockaddr_un *)sa;

		if (unp->sun_path[0] == '\0')
			strcpy(res, "(no pathname bound)");
		else
			snprintf(res, len, "%s", unp->sun_path);

		return res;
	}

	default: {
		snprintf(res, len,
			 "sock_ntop: unknown AF_xxx: %d, len: %d",
			 sa->sa_family, salen);

		return res;
	}

	}
}

int tst_getsockport(const char *file, const int lineno, int sockfd)
{
	struct sockaddr_storage ss;
	socklen_t addrlen = sizeof(ss);
	struct sockaddr *sa = (struct sockaddr *)&ss;

	safe_getsockname(file, lineno, NULL, sockfd, sa, &addrlen);

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in *sin = (struct sockaddr_in *)sa;

		return ntohs(sin->sin_port);
	}
	case AF_INET6: {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;

		return ntohs(sin6->sin6_port);
	} }

	return -1;
}

int safe_socket(const char *file, const int lineno, void (cleanup_fn)(void),
		int domain, int type, int protocol)
{
	int rval, ttype;

	rval = socket(domain, type, protocol);

	if (rval == -1) {
		switch (errno) {
		case EPROTONOSUPPORT:
		case ESOCKTNOSUPPORT:
		case EOPNOTSUPP:
		case EPFNOSUPPORT:
		case EAFNOSUPPORT:
			ttype = TCONF;
			break;
		default:
			ttype = TBROK;
		}

		tst_brkm_(file, lineno, ttype | TERRNO, cleanup_fn,
			"socket(%d, %d, %d) failed", domain, type, protocol);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid socket(%d, %d, %d) return value %d", domain,
			type, protocol, rval);
	}

	return rval;
}

int safe_socketpair(const char *file, const int lineno, int domain, int type,
		    int protocol, int sv[])
{
	int rval, ttype;

	rval = socketpair(domain, type, protocol, sv);

	if (rval == -1) {
		switch (errno) {
		case EPROTONOSUPPORT:
		case EOPNOTSUPP:
		case EAFNOSUPPORT:
			ttype = TCONF;
			break;
		default:
			ttype = TBROK;
		}

		tst_brkm_(file, lineno, ttype | TERRNO, NULL,
			"socketpair(%d, %d, %d, %p) failed", domain, type,
			protocol, sv);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid socketpair(%d, %d, %d, %p) return value %d",
			domain, type, protocol, sv, rval);
	}

	return rval;
}

int safe_getsockopt(const char *file, const int lineno, int sockfd, int level,
		    int optname, void *optval, socklen_t *optlen)
{
	int rval = getsockopt(sockfd, level, optname, optval, optlen);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"getsockopt(%d, %d, %d, %p, %p) failed",
			sockfd, level, optname, optval, optlen);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid getsockopt(%d, %d, %d, %p, %p) return value %d",
			sockfd, level, optname, optval, optlen, rval);
	}

	return rval;
}

int safe_setsockopt(const char *file, const int lineno, int sockfd, int level,
		    int optname, const void *optval, socklen_t optlen)
{
	int rval;

	rval = setsockopt(sockfd, level, optname, optval, optlen);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"setsockopt(%d, %d, %d, %p, %d) failed",
			sockfd, level, optname, optval, optlen);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid setsockopt(%d, %d, %d, %p, %d) return value %d",
			sockfd, level, optname, optval, optlen, rval);
	}

	return rval;
}

ssize_t safe_send(const char *file, const int lineno, char len_strict,
		  int sockfd, const void *buf, size_t len, int flags)
{
	ssize_t rval;

	rval = send(sockfd, buf, len, flags);

	if (rval == -1 || (len_strict && (size_t)rval != len)) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"send(%d, %p, %zu, %d) failed", sockfd, buf, len,
			flags);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid send(%d, %p, %zu, %d) return value %zd",
			sockfd, buf, len, flags, rval);
	}

	return rval;
}

ssize_t safe_sendto(const char *file, const int lineno, char len_strict,
		    int sockfd, const void *buf, size_t len, int flags,
		    const struct sockaddr *dest_addr, socklen_t addrlen)
{
	ssize_t rval;
	char res[128];

	rval = sendto(sockfd, buf, len, flags, dest_addr, addrlen);

	if (rval == -1 || (len_strict && (size_t)rval != len)) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"sendto(%d, %p, %zu, %d, %s, %d) failed",
			sockfd, buf, len, flags,
			tst_sock_addr(dest_addr, addrlen, res, sizeof(res)),
			addrlen);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid sendto(%d, %p, %zu, %d, %s, %d) return value %zd",
			sockfd, buf, len, flags,
			tst_sock_addr(dest_addr, addrlen, res, sizeof(res)),
			addrlen, rval);
	}

	return rval;
}

ssize_t safe_sendmsg(const char *file, const int lineno, size_t len,
		     int sockfd, const struct msghdr *msg, int flags)
{
	ssize_t rval;

	rval = sendmsg(sockfd, msg, flags);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"sendmsg(%d, %p, %d) failed", sockfd, msg, flags);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid sendmsg(%d, %p, %d) return value %zd",
			sockfd, msg, flags, rval);
	} else if (len && (size_t)rval != len) {
		tst_brkm_(file, lineno, TBROK, NULL,
			 "sendmsg(%d, %p, %d) ret(%zd) != len(%zu)",
			 sockfd, msg, flags, rval, len);
	}

	return rval;
}

ssize_t safe_recv(const char *file, const int lineno, size_t len,
	int sockfd, void *buf, size_t size, int flags)
{
	ssize_t rval;

	rval = recv(sockfd, buf, size, flags);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"recv(%d, %p, %zu, %d) failed", sockfd, buf, size,
			flags);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid recv(%d, %p, %zu, %d) return value %zd",
			sockfd, buf, size, flags, rval);
	} else if (len && (size_t)rval != len) {
		tst_brkm_(file, lineno, TBROK, NULL,
			"recv(%d, %p, %zu, %d) ret(%zd) != len(%zu)",
			sockfd, buf, size, flags, rval, len);
	}

	return rval;

}

ssize_t safe_recvmsg(const char *file, const int lineno, size_t len,
		     int sockfd, struct msghdr *msg, int flags)
{
	ssize_t rval;

	rval = recvmsg(sockfd, msg, flags);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"recvmsg(%d, %p, %d) failed", sockfd, msg, flags);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid recvmsg(%d, %p, %d) return value %zd",
			sockfd, msg, flags, rval);
	} else if (len && (size_t)rval != len) {
		tst_brkm_(file, lineno, TBROK, NULL,
			"recvmsg(%d, %p, %d) ret(%zd) != len(%zu)",
			sockfd, msg, flags, rval, len);
	}

	return rval;

}

int safe_bind(const char *file, const int lineno, void (cleanup_fn)(void),
	      int socket, const struct sockaddr *address,
	      socklen_t address_len)
{
	int i, ret;
	char buf[128];

	for (i = 0; i < 120; i++) {
		ret = bind(socket, address, address_len);

		if (!ret)
			return 0;

		if (ret != -1) {
			tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
				"Invalid bind(%d, %s, %d) return value %d",
				socket, tst_sock_addr(address, address_len,
				buf, sizeof(buf)), address_len, ret);
			return ret;
		} else if (errno != EADDRINUSE) {
			tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
				"bind(%d, %s, %d) failed", socket,
				tst_sock_addr(address, address_len, buf,
				sizeof(buf)), address_len);
			return ret;
		}

		if ((i + 1) % 10 == 0) {
			tst_resm_(file, lineno, TINFO,
				"address is in use, waited %3i sec", i + 1);
		}

		sleep(1);
	}

	tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
		"Failed to bind(%d, %s, %d) after 120 retries", socket,
		tst_sock_addr(address, address_len, buf, sizeof(buf)),
		address_len);
	return -1;
}

int safe_listen(const char *file, const int lineno, void (cleanup_fn)(void),
		int socket, int backlog)
{
	int rval;
	int res = TBROK;

	rval = listen(socket, backlog);

	if (rval == -1) {
		if (errno == ENOSYS)
			res = TCONF;
		tst_brkm_(file, lineno, res | TERRNO, cleanup_fn,
			"listen(%d, %d) failed", socket, backlog);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid listen(%d, %d) return value %d", socket,
			backlog, rval);
	}

	return rval;
}

int safe_accept(const char *file, const int lineno, void (cleanup_fn)(void),
		int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int rval;

	rval = accept(sockfd, addr, addrlen);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"accept(%d, %p, %d) failed", sockfd, addr, *addrlen);
	} else if (rval < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid accept(%d, %p, %d) return value %d", sockfd,
			addr, *addrlen, rval);
	}

	return rval;
}

int safe_connect(const char *file, const int lineno, void (cleanup_fn)(void),
		 int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int rval;
	char buf[128];

	rval = connect(sockfd, addr, addrlen);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"connect(%d, %s, %d) failed", sockfd,
			tst_sock_addr(addr, addrlen, buf, sizeof(buf)),
			addrlen);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid connect(%d, %s, %d) return value %d", sockfd,
			tst_sock_addr(addr, addrlen, buf, sizeof(buf)),
			addrlen, rval);
	}

	return rval;
}

int safe_getsockname(const char *file, const int lineno,
		     void (cleanup_fn)(void), int sockfd, struct sockaddr *addr,
		     socklen_t *addrlen)
{
	int rval;
	char buf[128];

	rval = getsockname(sockfd, addr, addrlen);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"getsockname(%d, %s, %d) failed", sockfd,
			tst_sock_addr(addr, *addrlen, buf, sizeof(buf)),
			*addrlen);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid getsockname(%d, %s, %d) return value %d",
			sockfd, tst_sock_addr(addr, *addrlen, buf,
			sizeof(buf)), *addrlen, rval);
	}

	return rval;
}

int safe_gethostname(const char *file, const int lineno,
		     char *name, size_t size)
{
	int rval = gethostname(name, size);

	if (rval == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"gethostname(%p, %zu) failed", name, size);
	} else if (rval) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"Invalid gethostname(%p, %zu) return value %d", name,
			size, rval);
	}

	return rval;
}

/*
 * @return port in network byte order.
 */
unsigned short tst_get_unused_port(const char *file, const int lineno,
	      void (cleanup_fn)(void), unsigned short family, int type)
{
	int sock, ret;
	socklen_t slen;
	struct sockaddr_storage _addr;
	struct sockaddr *addr = (struct sockaddr *)&_addr;
	struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
	struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;

	switch (family) {
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
		tst_brkm_(file, lineno, TBROK, cleanup_fn,
			"%s(): Unsupported socket family %d", __func__,
			family);
		return -1;
	}

	sock = safe_socket(file, lineno, cleanup_fn, addr->sa_family, type, 0);

	if (sock < 0)
		return sock;

	ret = safe_bind(file, lineno, cleanup_fn, sock, addr, slen);

	if (ret)
		return ret;

	ret = safe_getsockname(file, lineno, cleanup_fn, sock, addr, &slen);

	if (ret)
		return ret;

	ret = safe_close(file, lineno, cleanup_fn, sock);

	if (ret)
		return ret;

	switch (family) {
	case AF_INET:
		return addr4->sin_port;
	case AF_INET6:
		return addr6->sin6_port;
	default:
		return -1;
	}
}
