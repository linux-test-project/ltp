// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies the following shutdown() functionalities:
 *
 * - SHUT_RD should enable send() ops but disable recv() ops
 * - SHUT_WR should enable recv() ops but disable send() ops
 * - SHUT_RDWR should disable both recv() and send() ops
 */

#include "tst_test.h"
#include "tst_safe_net.h"

#define MSGSIZE 4
#define SOCKETFILE "socket"

#define OP_DESC(x) .shutdown_op = x, .desc = #x
static struct tcase {
	int shutdown_op;
	int recv_flag;
	int recv_err;
	int send_flag;
	int send_err;
	char *desc;
} tcases[] = {
	{OP_DESC(SHUT_RD)},
	{OP_DESC(SHUT_WR), .recv_flag = MSG_DONTWAIT, .recv_err = EWOULDBLOCK,
		.send_flag = MSG_NOSIGNAL, .send_err = EPIPE},
	{OP_DESC(SHUT_RDWR), .send_flag = MSG_NOSIGNAL, .send_err = EPIPE}
};

static struct sockaddr_un *sock_addr;

static void run_server(void)
{
	int server_sock;

	server_sock = SAFE_SOCKET(sock_addr->sun_family, SOCK_STREAM, 0);

	SAFE_BIND(server_sock,
		(struct sockaddr *)sock_addr,
		sizeof(struct sockaddr_un));
	SAFE_LISTEN(server_sock, 10);

	tst_res(TINFO, "Running server on socket file");

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_CLOSE(server_sock);
	SAFE_UNLINK(SOCKETFILE);
}

static int start_test(void)
{
	int client_sock;

	if (!SAFE_FORK()) {
		run_server();
		_exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Connecting to the server");

	client_sock = SAFE_SOCKET(sock_addr->sun_family, SOCK_STREAM, 0);
	SAFE_CONNECT(client_sock,
		(struct sockaddr *)sock_addr,
		sizeof(struct sockaddr_un));

	return client_sock;
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int client_sock;
	char buff[MSGSIZE] = {0};

	client_sock = start_test();

	tst_res(TINFO, "Testing %s flag", tc->desc);

	TST_EXP_PASS(shutdown(client_sock, tc->shutdown_op));

	if (tc->recv_err)
		TST_EXP_FAIL(recv(client_sock, buff, MSGSIZE, tc->recv_flag), tc->recv_err);
	else
		SAFE_RECV(0, client_sock, buff, MSGSIZE, tc->recv_flag);

	if (tc->send_err)
		TST_EXP_FAIL(send(client_sock, buff, MSGSIZE, tc->send_flag), tc->send_err);
	else
		SAFE_SEND(MSGSIZE, client_sock, buff, MSGSIZE, tc->send_flag);

	SAFE_CLOSE(client_sock);
	TST_CHECKPOINT_WAKE(0);
}

static void setup(void)
{
	sock_addr->sun_family = AF_UNIX;
	memcpy(sock_addr->sun_path, SOCKETFILE, sizeof(SOCKETFILE));
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&sock_addr, .size = sizeof(struct sockaddr_un)},
		{}
	}
};
