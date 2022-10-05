// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

/*
 * Create and bind socket for various standard datagram protocols.
 * Then connect to it and send some test data.
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "tst_test.h"
#include "tst_net.h"
#include "tst_safe_pthread.h"
#include "libbind.h"

static struct sockaddr_un unix_addr = {
	.sun_family = AF_UNIX,
	.sun_path = MAIN_SOCKET_FILE
};
static struct sockaddr_un abstract_addr = {
	.sun_family = AF_UNIX,
	.sun_path = ABSTRACT_SOCKET_PATH
};
static struct sockaddr_un peer_addr = {
	.sun_family = AF_UNIX,
	.sun_path = PEER_SOCKET_FILE
};
static struct sockaddr_in ipv4_addr;
static struct sockaddr_in ipv4_any_addr;
static struct sockaddr_in6 ipv6_addr;
static struct sockaddr_in6 ipv6_any_addr;

static struct test_case testcase_list[] = {
	/* UNIX sockets */
	{SOCK_DGRAM, 0, (struct sockaddr *)&unix_addr, sizeof(unix_addr),
		"AF_UNIX pathname datagram"},
	{SOCK_DGRAM, 0, (struct sockaddr *)&abstract_addr,
		sizeof(abstract_addr), "AF_UNIX abstract datagram"},

	/* IPv4 sockets */
	{SOCK_DGRAM, 0, (struct sockaddr *)&ipv4_addr, sizeof(ipv4_addr),
		"IPv4 loop UDP variant 1"},
	{SOCK_DGRAM, IPPROTO_UDP, (struct sockaddr *)&ipv4_addr,
		sizeof(ipv4_addr), "IPv4 loop UDP variant 2"},
	{SOCK_DGRAM, IPPROTO_UDPLITE, (struct sockaddr *)&ipv4_addr,
		sizeof(ipv4_addr), "IPv4 loop UDP-Lite"},
	{SOCK_DGRAM, 0, (struct sockaddr *)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any UDP variant 1"},
	{SOCK_DGRAM, IPPROTO_UDP, (struct sockaddr *)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any UDP variant 2"},
	{SOCK_DGRAM, IPPROTO_UDPLITE, (struct sockaddr *)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any UDP-Lite"},

	/* IPv6 sockets */
	{SOCK_DGRAM, 0, (struct sockaddr *)&ipv6_addr, sizeof(ipv6_addr),
		"IPv6 loop UDP variant 1"},
	{SOCK_DGRAM, IPPROTO_UDP, (struct sockaddr *)&ipv6_addr,
		sizeof(ipv6_addr), "IPv6 loop UDP variant 2"},
	{SOCK_DGRAM, IPPROTO_UDPLITE, (struct sockaddr *)&ipv6_addr,
		sizeof(ipv6_addr), "IPv6 loop UDP-Lite"},
	{SOCK_DGRAM, 0, (struct sockaddr *)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any UDP variant 1"},
	{SOCK_DGRAM, IPPROTO_UDP, (struct sockaddr *)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any UDP variant 2"},
	{SOCK_DGRAM, IPPROTO_UDPLITE, (struct sockaddr *)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any UDP-Lite"}
};

static void setup(void)
{
	srand(time(0));

	tst_init_sockaddr_inet(&ipv4_addr, IPV4_ADDRESS, 0);
	tst_init_sockaddr_inet_bin(&ipv4_any_addr, INADDR_ANY, 0);
	tst_init_sockaddr_inet6_bin(&ipv6_addr, &in6addr_loopback, 0);
	tst_init_sockaddr_inet6_bin(&ipv6_any_addr, &in6addr_any, 0);
}

static void *peer_thread(void *tc_ptr)
{
	const struct test_case *tc = tc_ptr;
	int sock;
	unsigned int request = 0;
	const char *response;

	sock = SAFE_SOCKET(tc->address->sa_family, tc->type, tc->protocol);

	/*
	 * Both sides of AF_UNIX/SOCK_DGRAM socket must be bound for
	 * bidirectional communication
	 */
	if (tc->address->sa_family == AF_UNIX)
		SAFE_BIND(sock, (struct sockaddr *)&peer_addr,
			sizeof(struct sockaddr_un));

	SAFE_CONNECT(sock, tc->address, tc->addrlen);
	SAFE_WRITE(SAFE_WRITE_ALL, sock, &request, sizeof(request));
	SAFE_READ(1, sock, &request, sizeof(request));

	if (request < ARRAY_SIZE(testcase_list))
		response = testcase_list[request].description;
	else
		response = "Invalid request value";

	SAFE_WRITE(SAFE_WRITE_ALL, sock, response, strlen(response) + 1);
	SAFE_CLOSE(sock);

	if (tc->address->sa_family == AF_UNIX)
		SAFE_UNLINK(PEER_SOCKET_FILE);

	return NULL;
}

static void test_bind(unsigned int n)
{
	struct test_case tc_copy, *tc = testcase_list + n;
	struct sockaddr_storage listen_addr, remote_addr;
	struct sockaddr_un *tmp_addr;
	socklen_t remote_len = sizeof(struct sockaddr_storage);
	int sock, size;
	unsigned int rand_index;
	pthread_t thread_id;
	char buffer[BUFFER_SIZE];
	const char *exp_data;

	tst_res(TINFO, "Testing %s", tc->description);
	sock = SAFE_SOCKET(tc->address->sa_family, tc->type, tc->protocol);

	TST_EXP_PASS_SILENT(bind(sock, tc->address, tc->addrlen), "bind()");

	if (!TST_PASS) {
		SAFE_CLOSE(sock);
		return;
	}

	/*
	 * IPv4/IPv6 tests use wildcard addresses, resolve a valid connection
	 * address for peer thread
	 */
	memcpy(&tc_copy, tc, sizeof(struct test_case));
	tc_copy.addrlen = tst_get_connect_address(sock, &listen_addr);
	tc_copy.address = (struct sockaddr *)&listen_addr;

	SAFE_PTHREAD_CREATE(&thread_id, NULL, peer_thread, &tc_copy);
	size = recvfrom(sock, &rand_index, sizeof(rand_index), 0,
		(struct sockaddr *)&remote_addr, &remote_len);

	if (size != sizeof(rand_index)) {
		SAFE_CLOSE(sock);
		tst_brk(TBROK | TERRNO, "Error while waiting for connection");
	}

	rand_index = rand() % ARRAY_SIZE(testcase_list);
	SAFE_SENDTO(1, sock, &rand_index, sizeof(rand_index), 0,
		(struct sockaddr *)&remote_addr, remote_len);

	size = SAFE_READ(0, sock, buffer, BUFFER_SIZE - 1);
	buffer[size] = '\0';
	exp_data = testcase_list[rand_index].description;

	if (!strcmp(buffer, exp_data))
		tst_res(TPASS, "Communication successful");
	else
		tst_res(TFAIL, "Received invalid data. Expected: \"%s\". "
			"Received: \"%s\"", exp_data, buffer);

	SAFE_CLOSE(sock);
	pthread_join(thread_id, NULL);
	tmp_addr = (struct sockaddr_un *)tc->address;

	if (tc->address->sa_family == AF_UNIX && tmp_addr->sun_path[0])
		SAFE_UNLINK(tmp_addr->sun_path);
}

static struct tst_test test = {
	.test = test_bind,
	.tcnt = ARRAY_SIZE(testcase_list),
	.needs_tmpdir = 1,
	.setup = setup,
};
