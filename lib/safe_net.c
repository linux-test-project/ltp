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

int safe_socket(const char *file, const int lineno, void (cleanup_fn)(void),
		int domain, int type, int protocol)
{
	int rval;

	rval = socket(domain, type, protocol);

	if (rval < 0) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: socket(%d, %d, %d) failed", file, lineno,
			 domain, type, protocol);
	}

	return rval;
}

int safe_setsockopt(const char *file, const int lineno, int sockfd, int level,
		    int optname, const void *optval, socklen_t optlen)
{
	int rval;

	rval = setsockopt(sockfd, level, optname, optval, optlen);

	if (rval) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: setsockopt(%d, %d, %d, %p, %d) failed",
			 file, lineno, sockfd, level, optname, optval, optlen);
	}

	return rval;
}

ssize_t safe_send(const char *file, const int lineno, char len_strict,
		  int sockfd, const void *buf, size_t len, int flags)
{
	ssize_t rval;

	rval = send(sockfd, buf, len, flags);

	if (rval == -1 || (len_strict && (size_t)rval != len)) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: send(%d, %p, %zu, %d) failed",
			 file, lineno, sockfd, buf, len, flags);
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
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: sendto(%d, %p, %zu, %d, %s, %d) failed",
			 file, lineno, sockfd, buf, len, flags,
			 tst_sock_addr(dest_addr, addrlen, res, sizeof(res)),
			 addrlen);
	}

	return rval;
}

int safe_bind(const char *file, const int lineno, void (cleanup_fn)(void),
	      int socket, const struct sockaddr *address,
	      socklen_t address_len)
{
	int i;
	char buf[128];

	for (i = 0; i < 120; i++) {
		if (!bind(socket, address, address_len))
			return 0;

		if (errno != EADDRINUSE) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				 "%s:%d: bind(%d, %s, %d) failed", file, lineno,
				 socket, tst_sock_addr(address, address_len,
						       buf, sizeof(buf)),
				 address_len);
			return -1;
		}

		if ((i + 1) % 10 == 0) {
			tst_resm(TINFO, "address is in use, waited %3i sec",
				 i + 1);
		}

		sleep(1);
	}

	tst_brkm(TBROK | TERRNO, cleanup_fn,
		 "%s:%d: Failed to bind(%d, %s, %d) after 120 retries", file,
		 lineno, socket,
		 tst_sock_addr(address, address_len, buf, sizeof(buf)),
		 address_len);
	return -1;
}

int safe_listen(const char *file, const int lineno, void (cleanup_fn)(void),
		int socket, int backlog)
{
	int rval;

	rval = listen(socket, backlog);

	if (rval < 0) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: listen(%d, %d) failed", file, lineno, socket,
			 backlog);
	}

	return rval;
}

int safe_connect(const char *file, const int lineno, void (cleanup_fn)(void),
		 int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int rval;
	char buf[128];

	rval = connect(sockfd, addr, addrlen);

	if (rval < 0) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: connect(%d, %s, %d) failed", file, lineno,
			 sockfd, tst_sock_addr(addr, addrlen, buf,
					       sizeof(buf)), addrlen);
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

	if (rval < 0) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: getsockname(%d, %s, %d) failed", file, lineno,
			 sockfd, tst_sock_addr(addr, *addrlen, buf,
					       sizeof(buf)), *addrlen);
	}

	return rval;
}

int safe_gethostname(const char *file, const int lineno,
		     char *name, size_t size)
{
	int rval = gethostname(name, size);

	if (rval < 0) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "%s:%d: gethostname(%p, %zu) failed",
			 file, lineno, name, size);
	}

	return rval;
}
