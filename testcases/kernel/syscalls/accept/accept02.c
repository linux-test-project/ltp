// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */
/* Test for CVE-2017-8890
 *
 * In Kernels up to 4.10.15 missing commit 657831ff the multicast
 * group information of a socket gets copied over to a newly created
 * socket when using the accept() syscall. This will cause a double free
 * when closing the original and the cloned socket.
 *
 * WARNING:
 * There is a high chance that this test will cause an unstable system
 * if it does not succeed!
 *
 * For more information about this CVE see:
 * https://www.suse.com/security/cve/CVE-2017-8890/
 */

#include <errno.h>
#include <sys/socket.h>
#include "tst_test.h"
#include "tst_safe_net.h"
#include "tst_safe_pthread.h"

#define MULTICASTIP "224.0.0.0"
#define LOCALHOSTIP "127.0.0.1"

static int server_sockfd;
static int clone_server_sockfd;
static int client_sockfd;
static int server_port;
static socklen_t addr_len;

static struct sockaddr_in *server_addr;
static struct sockaddr_in *client_addr;
static struct group_req *mc_group;

static void *server_thread(void *arg)
{
	int op, op_len, mc_group_len;

	op = 1;
	op_len = sizeof(op);
	mc_group_len = sizeof(*mc_group);

	server_sockfd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);

	SAFE_SETSOCKOPT(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &op, op_len);
	SAFE_SETSOCKOPT(server_sockfd, SOL_IP, MCAST_JOIN_GROUP,
			mc_group, mc_group_len);

	SAFE_BIND(server_sockfd, (struct sockaddr *)server_addr, addr_len);
	SAFE_LISTEN(server_sockfd, 1);

	TST_CHECKPOINT_WAKE(0);

	TEST(accept(server_sockfd, (struct sockaddr *)client_addr, &addr_len));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "Could not accept connection");

	clone_server_sockfd = TST_RET;

	TEST(setsockopt(clone_server_sockfd, SOL_IP, MCAST_LEAVE_GROUP,
			mc_group, mc_group_len));

	if (TST_RET != -1)
		tst_res(TFAIL, "Multicast group was copied!");
	else if (TST_ERR == EADDRNOTAVAIL)
		tst_res(TPASS | TTERRNO, "Multicast group was not copied");
	else
		tst_brk(TBROK | TTERRNO, "setsockopt() failed unexpectedly");

	SAFE_CLOSE(server_sockfd);
	return arg;
}

static void *client_thread(void *arg)
{
	client_sockfd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	SAFE_BIND(client_sockfd, (struct sockaddr *)client_addr, addr_len);

	SAFE_CONNECT(client_sockfd, (struct sockaddr *)server_addr, addr_len);

	SAFE_CLOSE(client_sockfd);
	return arg;
}

static void run(void)
{
	pthread_t server_thr, client_thr;

	server_addr->sin_port = server_port;
	client_addr->sin_port = htons(0);

	SAFE_PTHREAD_CREATE(&server_thr, NULL, server_thread, NULL);
	TST_CHECKPOINT_WAIT(0);
	SAFE_PTHREAD_CREATE(&client_thr, NULL, client_thread, NULL);

	SAFE_PTHREAD_JOIN(server_thr, NULL);
	SAFE_PTHREAD_JOIN(client_thr, NULL);
}

static void setup(void)
{
	struct sockaddr_in *mc_group_addr;

	server_addr = tst_alloc(sizeof(*server_addr));
	client_addr = tst_alloc(sizeof(*client_addr));
	mc_group = tst_alloc(sizeof(*mc_group));

	mc_group->gr_interface = 0;
	mc_group_addr = (struct sockaddr_in *) &mc_group->gr_group;
	mc_group_addr->sin_family = AF_INET;
	inet_aton(MULTICASTIP, &mc_group_addr->sin_addr);

	server_addr->sin_family = AF_INET;
	inet_aton(LOCALHOSTIP, &server_addr->sin_addr);

	client_addr->sin_family = AF_INET;
	client_addr->sin_addr.s_addr = htons(INADDR_ANY);

	addr_len = sizeof(struct sockaddr_in);

	server_port = TST_GET_UNUSED_PORT(AF_INET, SOCK_STREAM);
	tst_res(TINFO, "Starting listener on port: %d", ntohs(server_port));
}

static void cleanup(void)
{
	if (clone_server_sockfd > 0)
		SAFE_CLOSE(clone_server_sockfd);
	if (client_sockfd > 0)
		SAFE_CLOSE(client_sockfd);
	if (server_sockfd > 0)
		SAFE_CLOSE(server_sockfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_checkpoints = 1,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-8890"},
		{"linux-git", "657831ff"},
		{},
	}
};
