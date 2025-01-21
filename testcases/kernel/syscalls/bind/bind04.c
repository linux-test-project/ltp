// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

/*
 * Create and bind socket for various standard stream protocols.
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
static struct sockaddr_in ipv4_addr;
static struct sockaddr_in ipv4_any_addr;
static struct sockaddr_in6 ipv6_addr;
static struct sockaddr_in6 ipv6_any_addr;

static struct test_case testcase_list[] = {
	/* UNIX sockets */
	{SOCK_STREAM, 0, (struct sockaddr *)&unix_addr, sizeof(unix_addr),
		"AF_UNIX pathname stream"},
	{SOCK_SEQPACKET, 0, (struct sockaddr *)&unix_addr, sizeof(unix_addr),
		"AF_UNIX pathname seqpacket"},
	{SOCK_STREAM, 0, (struct sockaddr *)&abstract_addr,
		sizeof(abstract_addr), "AF_UNIX abstract stream"},
	{SOCK_SEQPACKET, 0, (struct sockaddr *)&abstract_addr,
		sizeof(abstract_addr), "AF_UNIX abstract seqpacket"},

	/* IPv4 sockets */
	{SOCK_STREAM, 0, (struct sockaddr *)&ipv4_addr, sizeof(ipv4_addr),
		"IPv4 loop TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr *)&ipv4_addr,
		sizeof(ipv4_addr), "IPv4 loop TCP variant 2"},
	{SOCK_STREAM, IPPROTO_SCTP, (struct sockaddr *)&ipv4_addr,
		sizeof(ipv4_addr), "IPv4 loop SCTP"},
	{SOCK_STREAM, 0, (struct sockaddr *)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr *)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any TCP variant 2"},
	{SOCK_STREAM, IPPROTO_SCTP, (struct sockaddr *)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any SCTP"},

	/* IPv6 sockets */
	{SOCK_STREAM, 0, (struct sockaddr *)&ipv6_addr, sizeof(ipv6_addr),
		"IPv6 loop TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr *)&ipv6_addr,
		sizeof(ipv6_addr), "IPv6 loop TCP variant 2"},
	{SOCK_STREAM, IPPROTO_SCTP, (struct sockaddr *)&ipv6_addr,
		sizeof(ipv6_addr), "IPv6 loop SCTP"},
	{SOCK_STREAM, 0, (struct sockaddr *)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr *)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any TCP variant 2"},
	{SOCK_STREAM, IPPROTO_SCTP, (struct sockaddr *)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any SCTP"}
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
	unsigned int request;
	const char *response;

	sock = SAFE_SOCKET(tc->address->sa_family, tc->type, tc->protocol);
	SAFE_CONNECT(sock, tc->address, tc->addrlen);
	SAFE_READ(1, sock, &request, sizeof(request));

	if (request < ARRAY_SIZE(testcase_list))
		response = testcase_list[request].description;
	else
		response = "Invalid request value";

	SAFE_WRITE(SAFE_WRITE_ALL, sock, response, strlen(response) + 1);
	SAFE_CLOSE(sock);
	return NULL;
}

static void test_bind(unsigned int n)
{
	struct test_case tc_copy, *tc = testcase_list + n;
	struct sockaddr_storage listen_addr, remote_addr;
	struct sockaddr_un *tmp_addr;
	socklen_t remote_len = sizeof(struct sockaddr_storage);
	int listen_sock, sock, size;
	unsigned int rand_index;
	pthread_t thread_id;
	char buffer[BUFFER_SIZE];
	const char *exp_data;

	tst_res(TINFO, "Testing %s", tc->description);
	listen_sock = SAFE_SOCKET(tc->address->sa_family, tc->type,
		tc->protocol);

	TST_EXP_PASS_SILENT(bind(listen_sock, tc->address, tc->addrlen), "bind()");

	if (!TST_PASS) {
		SAFE_CLOSE(listen_sock);
		return;
	}

	/*
	 * IPv4/IPv6 tests use wildcard addresses, resolve a valid connection
	 * address for peer thread
	 */
	memcpy(&tc_copy, tc, sizeof(struct test_case));
	tc_copy.addrlen = tst_get_connect_address(listen_sock, &listen_addr);
	tc_copy.address = (struct sockaddr *)&listen_addr;

	SAFE_LISTEN(listen_sock, 1);
	SAFE_PTHREAD_CREATE(&thread_id, NULL, peer_thread, &tc_copy);
	sock = SAFE_ACCEPT(listen_sock, (struct sockaddr *)&remote_addr,
		&remote_len);

	rand_index = rand() % ARRAY_SIZE(testcase_list);
	SAFE_WRITE(SAFE_WRITE_ALL, sock, &rand_index, sizeof(rand_index));

	size = SAFE_READ(0, sock, buffer, BUFFER_SIZE - 1);
	buffer[size] = '\0';
	exp_data = testcase_list[rand_index].description;

	if (!strcmp(buffer, exp_data))
		tst_res(TPASS, "Communication successful");
	else
		tst_res(TFAIL, "Received invalid data. Expected: \"%s\". "
			"Received: \"%s\"", exp_data, buffer);

	SAFE_CLOSE(sock);
	SAFE_CLOSE(listen_sock);
	pthread_join(thread_id, NULL);
	tmp_addr = (struct sockaddr_un *)tc->address;

	if (tc->address->sa_family == AF_UNIX && tmp_addr->sun_path[0])
		SAFE_UNLINK(tmp_addr->sun_path);
}

static struct tst_test test = {
	.timeout = 1,
	.test = test_bind,
	.tcnt = ARRAY_SIZE(testcase_list),
	.needs_tmpdir = 1,
	.setup = setup,
};
