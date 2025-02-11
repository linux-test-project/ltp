// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that epoll receives EPOLLRDHUP event when we hang a reading
 * half-socket we are polling on.
 */

#include "tst_test.h"
#include "tst_net.h"
#include "tst_epoll.h"

static int epfd;
static int sockfd_client;
static int sockfd_server;
static in_port_t *sock_port;

static void create_server(void)
{
	int sockfd_server;
	socklen_t len;
	struct sockaddr_in serv_addr;
	struct sockaddr_in sin;

	tst_init_sockaddr_inet_bin(&serv_addr, INADDR_ANY, 0);

	sockfd_server = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	SAFE_BIND(sockfd_server, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	SAFE_LISTEN(sockfd_server, 10);

	len = sizeof(sin);
	memset(&sin, 0, sizeof(struct sockaddr_in));
	SAFE_GETSOCKNAME(sockfd_server, (struct sockaddr *)&sin, &len);

	*sock_port = ntohs(sin.sin_port);

	tst_res(TINFO, "Listening on port %d", *sock_port);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_CLOSE(sockfd_server);
}

static void run(void)
{
	struct sockaddr_in client_addr;
	struct epoll_event evt_req;
	struct epoll_event evt_rec;
	int ret;

	if (!SAFE_FORK()) {
		create_server();
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Connecting to port %d", *sock_port);

	sockfd_client = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);

	tst_init_sockaddr_inet(&client_addr, "127.0.0.1", *sock_port);

	SAFE_CONNECT(sockfd_client,
		(struct sockaddr *)&client_addr,
		sizeof(client_addr));

	tst_res(TINFO, "Polling on socket");

	epfd = SAFE_EPOLL_CREATE1(0);
	evt_req.events = EPOLLRDHUP;
	SAFE_EPOLL_CTL(epfd, EPOLL_CTL_ADD, sockfd_client, &evt_req);

	tst_res(TINFO, "Hang socket");

	TST_EXP_PASS_SILENT(shutdown(sockfd_client, SHUT_RD));
	ret = SAFE_EPOLL_WAIT(epfd, &evt_rec, 1, 2000);
	if (ret != 1) {
		tst_res(TFAIL, "Wrong number of events reported %i", ret);
		goto exit;
	}

	if (evt_rec.events & EPOLLRDHUP)
		tst_res(TPASS, "Received EPOLLRDHUP");
	else
		tst_res(TFAIL, "EPOLLRDHUP has not been received");

exit:
	SAFE_CLOSE(epfd);
	SAFE_CLOSE(sockfd_client);

	TST_CHECKPOINT_WAKE(0);
}

static void setup(void)
{
	sock_port = SAFE_MMAP(NULL, sizeof(in_port_t), PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (sock_port)
		SAFE_MUNMAP(sock_port, sizeof(in_port_t));

	if (fcntl(sockfd_client, F_GETFD) > 0)
		SAFE_CLOSE(sockfd_client);

	if (fcntl(sockfd_server, F_GETFD) > 0)
		SAFE_CLOSE(sockfd_server);

	if (fcntl(epfd, F_GETFD) > 0)
		SAFE_CLOSE(epfd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
