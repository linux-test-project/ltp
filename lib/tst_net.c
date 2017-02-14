/*
 * Copyright (C) 2014 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation, Inc.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "test.h"

unsigned short tst_get_unused_port(void (cleanup_fn)(void),
	unsigned short family, int type)
{
	int sock;
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
		tst_brkm(TBROK, cleanup_fn,
			"tst_get_unused_port unknown family");
		return -1;
	}

	sock = socket(addr->sa_family, type, 0);
	if (sock < 0) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "socket failed");
		return -1;
	}

	if (bind(sock, addr, slen) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "bind failed");
		return -1;
	}

	if (getsockname(sock, addr, &slen) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "getsockname failed");
		return -1;
	}

	if (close(sock) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "close failed");
		return -1;
	}

	switch (family) {
	case AF_INET:
		return addr4->sin_port;
	case AF_INET6:
		return addr6->sin6_port;
	default:
		return -1;
	}
}
