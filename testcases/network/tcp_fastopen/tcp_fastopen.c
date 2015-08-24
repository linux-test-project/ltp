/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 */

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "lapi/posix_clocks.h"
#include "safe_macros.h"

char *TCID = "tcp_fastopen";

static const int max_msg_len = 1500;

/* TCP server requiers */
#ifndef TCP_FASTOPEN
#define TCP_FASTOPEN	23
#endif

#ifndef SO_BUSY_POLL
#define SO_BUSY_POLL	46
#endif

/* TCP client requiers */
#ifndef MSG_FASTOPEN
#define MSG_FASTOPEN	0x20000000 /* Send data in TCP SYN */
#endif

enum {
	TCP_SERVER = 0,
	TCP_CLIENT,
};
static int tcp_mode;

enum {
	TFO_ENABLED = 0,
	TFO_DISABLED,
};
static int tfo_support;
static int fastopen_api;

static const char tfo_cfg[]		= "/proc/sys/net/ipv4/tcp_fastopen";
static const char tcp_tw_reuse[]	= "/proc/sys/net/ipv4/tcp_tw_reuse";
static int tw_reuse_changed;
static int tfo_cfg_value;
static int tfo_bit_num;
static int tfo_cfg_changed;
static int tfo_queue_size	= 100;
static int max_queue_len	= 100;
static const int client_byte	= 0x43;
static const int server_byte	= 0x53;
static const int start_byte	= 0x24;
static const int start_fin_byte	= 0x25;
static const int end_byte	= 0x0a;
static int client_msg_size	= 32;
static int server_msg_size	= 128;
static char *client_msg;
static char *server_msg;

/*
 * The number of requests from client after
 *  which server has to close the connection.
 */
static int server_max_requests	= 3;
static int client_max_requests	= 10;
static int clients_num		= 2;
static char *tcp_port		= "61000";
static char *server_addr	= "localhost";
static int busy_poll		= -1;
/* server socket */
static int sfd;

/* how long a client must wait for the server's reply, microsec */
static long wait_timeout = 10000000;

/* in the end test will save time result in this file */
static char *rpath		= "./tfo_result";

static int force_run;
static int verbose;

static char *narg, *Narg, *qarg, *rarg, *Rarg, *aarg, *Targ, *barg;

static const option_t options[] = {
	/* server params */
	{"R:", NULL, &Rarg},
	{"q:", NULL, &qarg},

	/* client params */
	{"H:", NULL, &server_addr},
	{"a:", NULL, &aarg},
	{"n:", NULL, &narg},
	{"N:", NULL, &Narg},
	{"T:", NULL, &Targ},
	{"r:", NULL, &rarg},
	{"d:", NULL, &rpath},

	/* common */
	{"g:", NULL, &tcp_port},
	{"b:", NULL, &barg},
	{"F", &force_run, NULL},
	{"l", &tcp_mode, NULL},
	{"o", &fastopen_api, NULL},
	{"O", &tfo_support, NULL},
	{"v", &verbose, NULL},
	{NULL, NULL, NULL}
};

static void help(void)
{
	printf("\n  -F      Force to run\n");
	printf("  -v      Verbose\n");
	printf("  -o      Use old TCP API, default is new TCP API\n");
	printf("  -O      TFO support is off, default is on\n");
	printf("  -l      Become TCP Client, default is TCP server\n");
	printf("  -g x    x - server port, default is %s\n", tcp_port);
	printf("  -b x    x - low latency busy poll timeout\n");

	printf("\n          Client:\n");
	printf("  -H x    x - server name or ip address, default is '%s'\n",
		server_addr);
	printf("  -a x    x - num of clients running in parallel\n");
	printf("  -r x    x - num of client requests\n");
	printf("  -n x    Client message size, max msg size is '%d'\n",
		max_msg_len);
	printf("  -N x    Server message size, max msg size is '%d'\n",
		max_msg_len);
	printf("  -T x    Reply timeout, default is '%ld' (microsec)\n",
		wait_timeout);
	printf("  -d x    x is a path to the file where results are saved\n");

	printf("\n          Server:\n");
	printf("  -R x    x - num of requests, after which conn. closed\n");
	printf("  -q x    x - server's limit on the queue of TFO requests\n");
}

/* common structure for TCP server and TCP client */
struct tcp_func {
	void (*init)(void);
	void (*run)(void);
	void (*cleanup)(void);
};
static struct tcp_func tcp;

#define MAX_THREADS	10000
static pthread_attr_t attr;
static pthread_t *thread_ids;

static struct addrinfo *remote_addrinfo;
static struct addrinfo *local_addrinfo;
static const struct linger clo = { 1, 3 };

static void do_cleanup(void)
{
	free(client_msg);
	free(server_msg);

	tcp.cleanup();

	if (tfo_cfg_changed) {
		SAFE_FILE_SCANF(NULL, tfo_cfg, "%d", &tfo_cfg_value);
		tfo_cfg_value &= ~tfo_bit_num;
		tfo_cfg_value |= !tfo_support << (tfo_bit_num - 1);
		tst_resm(TINFO, "unset '%s' back to '%d'",
			tfo_cfg, tfo_cfg_value);
		SAFE_FILE_PRINTF(NULL, tfo_cfg, "%d", tfo_cfg_value);
	}

	if (tw_reuse_changed) {
		SAFE_FILE_PRINTF(NULL, tcp_tw_reuse, "0");
		tst_resm(TINFO, "unset '%s' back to '0'", tcp_tw_reuse);
	}
}
TST_DECLARE_ONCE_FN(cleanup, do_cleanup)

static int sock_recv_poll(int fd, char *buf, int buf_size, int offset)
{
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	int len = -1;
	while (1) {
		errno = 0;
		int ret = poll(&pfd, 1, wait_timeout / 1000);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			break;
		}

		if (ret == 0) {
			errno = ETIME;
			break;
		}

		if (ret != 1 || !(pfd.revents & POLLIN))
			break;

		errno = 0;
		len = recv(fd, buf + offset,
			buf_size - offset, MSG_DONTWAIT);

		if (len == -1 && errno == EINTR)
			continue;
		else
			break;
	}

	return len;
}

static int client_recv(int *fd, char *buf)
{
	int len, offset = 0;

	while (1) {
		errno = 0;
		len = sock_recv_poll(*fd, buf, server_msg_size, offset);

		/* socket closed or msg is not valid */
		if (len < 1 || (offset + len) > server_msg_size ||
		   (buf[0] != start_byte && buf[0] != start_fin_byte)) {
			if (!errno)
				errno = ENOMSG;
			break;
		}
		offset += len;
		if (buf[offset - 1] != end_byte)
			continue;

		if (verbose) {
			tst_resm_hexd(TINFO, buf, offset,
				"msg recv from sock %d:", *fd);
		}

		/* recv last msg, close socket */
		if (buf[0] == start_fin_byte)
			break;
		return 0;
	}

	shutdown(*fd, SHUT_WR);
	SAFE_CLOSE(cleanup, *fd);
	*fd = -1;
	return (errno) ? -1 : 0;
}

static int client_connect_send(const char *msg, int size)
{
	int cfd = socket(remote_addrinfo->ai_family, SOCK_STREAM, 0);
	const int flag = 1;

	if (cfd == -1)
		return cfd;

	setsockopt(cfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	if (busy_poll >= 0) {
		setsockopt(cfd, SOL_SOCKET, SO_BUSY_POLL,
			   &busy_poll, sizeof(busy_poll));
	}

	if (fastopen_api == TFO_ENABLED) {
		/* Replaces connect() + send()/write() */
		if (sendto(cfd, msg, size, MSG_FASTOPEN | MSG_NOSIGNAL,
		    remote_addrinfo->ai_addr,
		    remote_addrinfo->ai_addrlen) != size) {
			SAFE_CLOSE(cleanup, cfd);
			return -1;
		}
	} else {
		/* old TCP API */
		if (connect(cfd, remote_addrinfo->ai_addr,
		    remote_addrinfo->ai_addrlen)) {
			SAFE_CLOSE(cleanup, cfd);
			return -1;
		}

		if (send(cfd, msg, size, MSG_NOSIGNAL) != client_msg_size) {
			SAFE_CLOSE(cleanup, cfd);
			return -1;
		}
	}

	return cfd;
}

void *client_fn(LTP_ATTRIBUTE_UNUSED void *arg)
{
	char buf[server_msg_size];
	int cfd, i;
	intptr_t err = 0;

	/* connect & send requests */
	cfd = client_connect_send(client_msg, client_msg_size);
	if (cfd == -1) {
		err = errno;
		goto out;
	}

	if (client_recv(&cfd, buf)) {
		err = errno;
		goto out;
	}

	for (i = 1; i < client_max_requests; ++i) {

		/* check connection, it can be closed */
		int ret = 0;
		if (cfd != -1)
			ret = recv(cfd, buf, 1, MSG_DONTWAIT);

		if (ret == 0) {
			/* try to reconnect and send */
			if (cfd != -1)
				SAFE_CLOSE(cleanup, cfd);

			cfd = client_connect_send(client_msg, client_msg_size);
			if (cfd == -1) {
				err = errno;
				goto out;
			}

			if (client_recv(&cfd, buf)) {
				err = errno;
				break;
			}

			continue;

		} else if (ret > 0) {
			err = EMSGSIZE;
			break;
		}

		if (verbose) {
			tst_resm_hexd(TINFO, client_msg, client_msg_size,
				"try to send msg[%d]", i);
		}

		if (send(cfd, client_msg, client_msg_size,
			MSG_NOSIGNAL) != client_msg_size) {
			err = ECOMM;
			break;
		}
		if (client_recv(&cfd, buf)) {
			err = errno;
			break;
		}
	}

	if (cfd != -1)
		SAFE_CLOSE(cleanup, cfd);

out:
	return (void *) err;
}

union net_size_field {
	char bytes[2];
	uint16_t value;
};

static void make_client_request(void)
{
	client_msg[0] = start_byte;

	/* set size for reply */
	union net_size_field net_size;
	net_size.value = htons(server_msg_size);
	client_msg[1] = net_size.bytes[0];
	client_msg[2] = net_size.bytes[1];

	client_msg[client_msg_size - 1] = end_byte;
}

static int parse_client_request(const char *msg)
{
	union net_size_field net_size;
	net_size.bytes[0] = msg[1];
	net_size.bytes[1] = msg[2];
	int size = ntohs(net_size.value);
	if (size < 2 || size > max_msg_len)
		return -1;

	return size;
}

static struct timespec tv_client_start;
static struct timespec tv_client_end;

static void client_init(void)
{
	if (clients_num >= MAX_THREADS) {
		tst_brkm(TBROK, cleanup,
			"Unexpected num of clients '%d'",
			clients_num);
	}

	thread_ids = SAFE_MALLOC(NULL, sizeof(pthread_t) * clients_num);

	client_msg = SAFE_MALLOC(NULL, client_msg_size);
	memset(client_msg, client_byte, client_msg_size);

	make_client_request();

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	int err = getaddrinfo(server_addr, tcp_port, &hints, &remote_addrinfo);
	if (err) {
		tst_brkm(TBROK, cleanup, "getaddrinfo of '%s' failed, %s",
			server_addr, gai_strerror(err));
	}

	tst_resm(TINFO, "TCP Fast Open over IPv%s",
		(remote_addrinfo->ai_family == AF_INET6) ? "6" : "4");

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv_client_start);
	int i;
	for (i = 0; i < clients_num; ++i) {
		if (pthread_create(&thread_ids[i], 0, client_fn, NULL) != 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
				"pthread_create failed at %s:%d",
				__FILE__, __LINE__);
		}
	}
}

static void client_run(void)
{
	void *res = NULL;
	long clnt_time = 0;
	int i;
	for (i = 0; i < clients_num; ++i) {
		pthread_join(thread_ids[i], &res);
		if (res) {
			tst_brkm(TBROK, cleanup, "client[%d] failed: %s",
				i, strerror((intptr_t)res));
		}
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv_client_end);
	clnt_time = (tv_client_end.tv_sec - tv_client_start.tv_sec) * 1000 +
		(tv_client_end.tv_nsec - tv_client_start.tv_nsec) / 1000000;

	tst_resm(TINFO, "total time '%ld' ms", clnt_time);

	/* ask server to terminate */
	client_msg[0] = start_fin_byte;
	int cfd = client_connect_send(client_msg, client_msg_size);
	if (cfd != -1) {
		shutdown(cfd, SHUT_WR);
		SAFE_CLOSE(NULL, cfd);
	}
	/* the script tcp_fastopen_run.sh will remove it */
	SAFE_FILE_PRINTF(cleanup, rpath, "%ld", clnt_time);
}

static void client_cleanup(void)
{
	free(thread_ids);

	if (remote_addrinfo)
		freeaddrinfo(remote_addrinfo);
}

static char *make_server_reply(int size)
{
	char *send_msg = SAFE_MALLOC(NULL, size);
	memset(send_msg, server_byte, size - 1);
	send_msg[0] = start_byte;
	send_msg[size - 1] = end_byte;
	return send_msg;
}

void *server_fn(void *cfd)
{
	int client_fd = (intptr_t) cfd;
	int num_requests = 0, offset = 0;

	/* Reply will be constructed from first client request */
	char *send_msg = NULL;
	int send_msg_size = 0;

	char recv_msg[max_msg_len];

	setsockopt(client_fd, SOL_SOCKET, SO_LINGER, &clo, sizeof(clo));
	if (busy_poll >= 0) {
		setsockopt(client_fd, SOL_SOCKET, SO_BUSY_POLL,
			   &busy_poll, sizeof(busy_poll));
	}

	ssize_t recv_len;

	while (1) {
		recv_len = sock_recv_poll(client_fd, recv_msg,
			max_msg_len, offset);

		if (recv_len == 0)
			break;

		if (recv_len < 0 || (offset + recv_len) > max_msg_len ||
		   (recv_msg[0] != start_byte &&
		    recv_msg[0] != start_fin_byte)) {
			tst_resm(TFAIL, "recv failed, sock '%d'", client_fd);
			goto out;
		}

		offset += recv_len;

		if (recv_msg[offset - 1] != end_byte) {
			/* msg is not complete, continue recv */
			continue;
		}

		/* client asks to terminate */
		if (recv_msg[0] == start_fin_byte)
			goto out;

		if (verbose) {
			tst_resm_hexd(TINFO, recv_msg, offset,
				"msg recv from sock %d:", client_fd);
		}

		/* if we send reply for the first time, construct it here */
		if (!send_msg) {
			send_msg_size = parse_client_request(recv_msg);
			if (send_msg_size < 0) {
				tst_resm(TFAIL, "wrong msg size '%d'",
					send_msg_size);
				goto out;
			}
			send_msg = make_server_reply(send_msg_size);
		}

		/*
		 * It will tell client that server is going
		 * to close this connection.
		 */
		if (++num_requests >= server_max_requests)
			send_msg[0] = start_fin_byte;

		if (send(client_fd, send_msg, send_msg_size,
		    MSG_NOSIGNAL) == -1) {
			tst_resm(TFAIL | TERRNO, "send failed");
			goto out;
		}

		offset = 0;

		if (num_requests >= server_max_requests) {
			/* max reqs, close socket */
			shutdown(client_fd, SHUT_WR);
			break;
		}
	}

	free(send_msg);
	SAFE_CLOSE(cleanup, client_fd);
	return NULL;

out:
	free(send_msg);
	SAFE_CLOSE(cleanup, client_fd);
	cleanup();
	tst_exit();
}

static void server_thread_add(intptr_t client_fd)
{
	pthread_t id;
	if (pthread_create(&id, &attr, server_fn, (void *) client_fd)) {
		tst_brkm(TBROK | TERRNO, cleanup,
			"pthread_create failed at %s:%d", __FILE__, __LINE__);
	}
}

static void server_init(void)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, tcp_port, &hints, &local_addrinfo) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "getaddrinfo failed");

	/* IPv6 socket is also able to access IPv4 protocol stack */
	sfd = SAFE_SOCKET(cleanup, AF_INET6, SOCK_STREAM, 0);

	tst_resm(TINFO, "assigning a name to the server socket...");
	if (!local_addrinfo)
		tst_brkm(TBROK, cleanup, "failed to get the address");

	SAFE_BIND(cleanup, sfd, local_addrinfo->ai_addr,
		local_addrinfo->ai_addrlen);

	freeaddrinfo(local_addrinfo);

	const int flag = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	if (fastopen_api == TFO_ENABLED) {
		if (setsockopt(sfd, IPPROTO_TCP, TCP_FASTOPEN, &tfo_queue_size,
			sizeof(tfo_queue_size)) == -1)
			tst_brkm(TBROK, cleanup, "Can't set TFO sock. options");
	}

	SAFE_LISTEN(cleanup, sfd, max_queue_len);
	tst_resm(TINFO, "Listen on the socket '%d', port '%s'", sfd, tcp_port);
}

static void server_cleanup(void)
{
	SAFE_CLOSE(NULL, sfd);
}

static void server_run(void)
{
	/* IPv4 source address will be mapped to IPv6 address */
	struct sockaddr_in6 addr6;
	socklen_t addr_size = sizeof(addr6);

	pthread_attr_init(&attr);

	/*
	 * detaching threads allow to reclaim thread's resources
	 * once a thread finishes its work.
	 */
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "setdetachstate failed");

	while (1) {
		int client_fd = accept(sfd, (struct sockaddr *)&addr6,
			&addr_size);

		if (client_fd == -1)
			tst_brkm(TBROK, cleanup, "Can't create client socket");

		if (verbose) {
			char addr_buf[INET6_ADDRSTRLEN];
			tst_resm(TINFO, "conn: port '%d', addr '%s'",
				addr6.sin6_port, inet_ntop(AF_INET6,
				&addr6.sin6_addr, addr_buf, INET6_ADDRSTRLEN));
		}

		server_thread_add(client_fd);
	}
}

static void check_opt(const char *name, char *arg, int *val, int lim)
{
	if (arg) {
		if (sscanf(arg, "%i", val) != 1)
			tst_brkm(TBROK, NULL, "-%s option arg is not a number",
				 name);
		if (*val < lim)
			tst_brkm(TBROK, NULL, "-%s option arg is less than %d",
				name, lim);
	}
}

static void check_opt_l(const char *name, char *arg, long *val, long lim)
{
	if (arg) {
		if (sscanf(arg, "%ld", val) != 1)
			tst_brkm(TBROK, NULL, "-%s option arg is not a number",
				 name);
		if (*val < lim)
			tst_brkm(TBROK, NULL, "-%s option arg is less than %ld",
				name, lim);
	}
}

static void setup(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, help);

	/* if client_num is not set, use num of processors */
	clients_num = sysconf(_SC_NPROCESSORS_ONLN);

	check_opt("a", aarg, &clients_num, 1);
	check_opt("r", rarg, &client_max_requests, 1);
	check_opt("R", Rarg, &server_max_requests, 1);
	check_opt("n", narg, &client_msg_size, 1);
	check_opt("N", Narg, &server_msg_size, 1);
	check_opt("q", qarg, &tfo_queue_size, 1);
	check_opt_l("T", Targ, &wait_timeout, 0L);
	check_opt("b", barg, &busy_poll, 0);

	if (!force_run)
		tst_require_root();

	if (!force_run && tst_kvercmp(3, 7, 0) < 0) {
		tst_brkm(TCONF, NULL,
			"Test must be run with kernel 3.7 or newer");
	}

	if (!force_run && busy_poll >= 0 && tst_kvercmp(3, 11, 0) < 0) {
		tst_brkm(TCONF, NULL,
			"Test must be run with kernel 3.11 or newer");
	}

	/* check tcp fast open knob */
	if (!force_run && access(tfo_cfg, F_OK) == -1)
		tst_brkm(TCONF, NULL, "Failed to find '%s'", tfo_cfg);

	if (!force_run) {
		SAFE_FILE_SCANF(NULL, tfo_cfg, "%d", &tfo_cfg_value);
		tst_resm(TINFO, "'%s' is %d", tfo_cfg, tfo_cfg_value);
	}

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_resm(TINFO, "TCP %s is using %s TCP API.",
		(tcp_mode == TCP_SERVER) ? "server" : "client",
		(fastopen_api == TFO_ENABLED) ? "Fastopen" : "old");

	switch (tcp_mode) {
	case TCP_SERVER:
		tst_resm(TINFO, "max requests '%d'",
			server_max_requests);
		tcp.init	= server_init;
		tcp.run		= server_run;
		tcp.cleanup	= server_cleanup;
		tfo_bit_num = 2;
	break;
	case TCP_CLIENT:
		tst_resm(TINFO, "connection: addr '%s', port '%s'",
			server_addr, tcp_port);
		tst_resm(TINFO, "client max req: %d", client_max_requests);
		tst_resm(TINFO, "clients num: %d", clients_num);
		tst_resm(TINFO, "client msg size: %d", client_msg_size);
		tst_resm(TINFO, "server msg size: %d", server_msg_size);

		tcp.init	= client_init;
		tcp.run		= client_run;
		tcp.cleanup	= client_cleanup;
		tfo_bit_num = 1;
	break;
	}

	tfo_support = TFO_ENABLED == tfo_support;
	if (((tfo_cfg_value & tfo_bit_num) == tfo_bit_num) != tfo_support) {
		int value = (tfo_cfg_value & ~tfo_bit_num)
			| (tfo_support << (tfo_bit_num - 1));
		tst_resm(TINFO, "set '%s' to '%d'", tfo_cfg, value);
		SAFE_FILE_PRINTF(cleanup, tfo_cfg, "%d", value);
		tfo_cfg_changed = 1;
	}

	int reuse_value = 0;
	SAFE_FILE_SCANF(cleanup, tcp_tw_reuse, "%d", &reuse_value);
	if (!reuse_value) {
		SAFE_FILE_PRINTF(cleanup, tcp_tw_reuse, "1");
		tw_reuse_changed = 1;
		tst_resm(TINFO, "set '%s' to '1'", tcp_tw_reuse);
	}

	tst_resm(TINFO, "TFO support %s",
		(tfo_support) ? "enabled" : "disabled");

	tcp.init();
}

int main(int argc, char *argv[])
{
	setup(argc, argv);

	tcp.run();

	cleanup();

	tst_exit();
}
