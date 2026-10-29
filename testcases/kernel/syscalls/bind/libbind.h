/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

/*
 * Common settings and data types for bind() connection tests
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define MAIN_SOCKET_FILE "test.sock"
#define ABSTRACT_SOCKET_PATH "\0test.sock"
#define PEER_SOCKET_FILE "peer.sock"
#define IPV4_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 128

struct test_case {
	int type, protocol;
	struct sockaddr *address;
	socklen_t addrlen;
	const char *description;
};
