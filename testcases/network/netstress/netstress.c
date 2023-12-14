// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2016 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2014-2023 Petr Vorel <pvorel@suse.cz>
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 */

#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <linux/dccp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "lapi/udp.h"
#include "lapi/dccp.h"
#include "lapi/netinet_in.h"
#include "lapi/posix_clocks.h"
#include "lapi/socket.h"
#include "lapi/tcp.h"
#include "tst_safe_stdio.h"
#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "tst_safe_net.h"

#if !defined(HAVE_RAND_R)
static int rand_r(LTP_ATTRIBUTE_UNUSED unsigned int *seed)
{
    return rand();
}
#endif

static const int max_msg_len = (1 << 16) - 1;
static const int min_msg_len = 5;

enum {
	SERVER_HOST = 0,
	CLIENT_HOST,
};
static char *client_mode;

enum {
	TFO_DISABLED = 0,
	TFO_ENABLED,
};
static int tfo_value = -1;
static char *fastopen_api, *fastopen_sapi;

static const char tfo_cfg[]		= "/proc/sys/net/ipv4/tcp_fastopen";
static const char tcp_tw_reuse[]	= "/proc/sys/net/ipv4/tcp_tw_reuse";
static int tw_reuse_changed;
static int tfo_cfg_value;
static int tfo_cfg_changed;
static int tfo_queue_size	= 100;
static int max_queue_len	= 100;
static const int client_byte	= 0x43;
static const int server_byte	= 0x53;
static const int start_byte	= 0x24;
static const int start_fin_byte	= 0x25;
static const int end_byte	= 0x0a;
static int init_cln_msg_len	= 32;
static int init_srv_msg_len	= 128;
static int max_rand_msg_len;
static int init_seed;

/*
 * The number of requests from client after
 * which server has to close the connection.
 */
static int server_max_requests	= 3;
static int client_max_requests	= 10;
static int clients_num;
static char *tcp_port;
static char *server_addr;
static char *source_addr;
static char *server_bg;
static int busy_poll		= -1;
static int max_etime_cnt = 21; /* ~60 sec max timeout if no connection */
static int max_eshutdown_cnt = 10;
static int max_pmtu_err = 10;

enum {
	TYPE_TCP = 0,
	TYPE_UDP,
	TYPE_UDP_LITE,
	TYPE_DCCP,
	TYPE_SCTP
};
static uint proto_type;
static char *type;
static char *dev;
static int sock_type = SOCK_STREAM;
static int protocol;
static int family = AF_INET6;

static uint32_t service_code = 0xffff;

/* server socket */
static int sfd;

/* how long a client must wait for the server's reply */
static int wait_timeout = 60000;

/* in the end test will save time result in this file */
static char *rpath;
static char *port_path = "netstress_port";
static char *log_path = "netstress.log";

static char *narg, *Narg, *qarg, *rarg, *Rarg, *aarg, *Targ, *barg, *targ,
	    *Aarg;

/* common structure for TCP/UDP server and TCP/UDP client */
struct net_func {
	void (*init)(void);
	void (*run)(void);
	void (*cleanup)(void);
};
static struct net_func net;

#define MAX_THREADS	10000
static pthread_attr_t attr;
static pthread_t *thread_ids;

static struct addrinfo *remote_addrinfo;
static struct addrinfo *local_addrinfo;

struct sock_info {
	int fd;
	struct sockaddr_storage raddr;
	socklen_t raddr_len;
	int etime_cnt;
	int pmtu_err_cnt;
	int eshutdown_cnt;
	int timeout;
};

static char *zcopy;
static int send_flags = MSG_NOSIGNAL;
static char *reuse_port;

static void init_socket_opts(int sd)
{
	if (busy_poll >= 0)
		SAFE_SETSOCKOPT_INT(sd, SOL_SOCKET, SO_BUSY_POLL, busy_poll);

	if (dev)
		SAFE_SETSOCKOPT(sd, SOL_SOCKET, SO_BINDTODEVICE, dev,
				strlen(dev) + 1);

	switch (proto_type) {
	case TYPE_TCP:
		if (client_mode && fastopen_sapi) {
			SAFE_SETSOCKOPT_INT(sd, IPPROTO_TCP,
					    TCP_FASTOPEN_CONNECT, 1);
		}
		if (client_mode && zcopy)
			SAFE_SETSOCKOPT_INT(sd, SOL_SOCKET, SO_ZEROCOPY, 1);
	break;
	case TYPE_DCCP:
		SAFE_SETSOCKOPT_INT(sd, SOL_DCCP, DCCP_SOCKOPT_SERVICE,
				    service_code);
	break;
	case TYPE_UDP_LITE: {
		int cscov = init_srv_msg_len >> 1;

		if (cscov < 8)
			cscov = 8;
		tst_res(TINFO, "UDP-Lite send cscov is %d", cscov);
		/* set checksum for header and partially for payload */
		SAFE_SETSOCKOPT_INT(sd, SOL_UDPLITE, UDPLITE_SEND_CSCOV, cscov);
		SAFE_SETSOCKOPT_INT(sd, SOL_UDPLITE, UDPLITE_RECV_CSCOV, 8);
	} break;
	}
}

static void do_cleanup(void)
{
	if (net.cleanup)
		net.cleanup();

	if (tfo_cfg_changed) {
		tst_res(TINFO, "unset '%s' back to '%d'",
			tfo_cfg, tfo_cfg_value);
		SAFE_FILE_PRINTF(tfo_cfg, "%d", tfo_cfg_value);
	}

	if (tw_reuse_changed) {
		SAFE_FILE_PRINTF(tcp_tw_reuse, "0");
		tst_res(TINFO, "unset '%s' back to '0'", tcp_tw_reuse);
	}
}
TST_DECLARE_ONCE_FN(cleanup, do_cleanup)

static int sock_recv_poll(char *buf, int size, struct sock_info *i)
{
	struct pollfd pfd;
	pfd.fd = i->fd;
	pfd.events = POLLIN;
	int len = -1;

	while (1) {
		errno = 0;
		int ret = poll(&pfd, 1, i->timeout);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			break;
		}

		if (ret != 1) {
			if (!errno)
				errno = ETIME;
			break;
		}

		if (!(pfd.revents & POLLIN)) {
			if (pfd.revents & POLLERR) {
				int err = 0;
				socklen_t err_len = sizeof(err);

				getsockopt(i->fd, SOL_SOCKET, SO_ERROR,
					   &err, &err_len);
				if (!err)
					continue;
				errno = err;
			}
			break;
		}

		errno = 0;
		len = recvfrom(i->fd, buf, size, MSG_DONTWAIT,
			       (struct sockaddr *)&i->raddr,
			       &i->raddr_len);

		if (len == -1 && errno == EINTR)
			continue;

		if (len == 0)
			errno = ESHUTDOWN;

		break;
	}

	return len;
}

static int client_recv(char *buf, int srv_msg_len, struct sock_info *i)
{
	int len, offset = 0;

	while (1) {
		errno = 0;
		len = sock_recv_poll(buf + offset, srv_msg_len - offset, i);

		/* socket closed or msg is not valid */
		if (len < 1 || (offset + len) > srv_msg_len ||
		   (buf[0] != start_byte && buf[0] != start_fin_byte)) {
			/* packet too big message, resend with new pmtu */
			if (errno == EMSGSIZE) {
				if (++(i->pmtu_err_cnt) < max_pmtu_err)
					return 0;
				tst_brk(TFAIL, "too many pmtu errors %d",
					i->pmtu_err_cnt);
			} else if (!errno) {
				errno = ENOMSG;
			}
			break;
		}
		offset += len;
		if (buf[offset - 1] != end_byte)
			continue;

		/* recv last msg, close socket */
		if (buf[0] == start_fin_byte)
			break;
		return 0;
	}

	if (sock_type != SOCK_STREAM) {
		if (errno == ETIME) {
			if (++(i->etime_cnt) > max_etime_cnt)
				tst_brk(TFAIL, "client requests timeout %d times, last timeout %dms",
					i->etime_cnt, i->timeout);
			/* Increase timeout in poll up to 3.2 sec */
			if (i->timeout < 3000)
				i->timeout <<= 1;
			return 0;
		}
		if (errno == ESHUTDOWN) {
			if (++(i->eshutdown_cnt) > max_eshutdown_cnt)
				tst_brk(TFAIL, "too many zero-length msgs");
			tst_res(TINFO, "%d-length msg on sock %d", len, i->fd);
			return 0;
		}
	}

	SAFE_CLOSE(i->fd);
	return (errno) ? -1 : 0;
}

static int bind_no_port;
static void bind_before_connect(int sd)
{
	if (!local_addrinfo)
		return;

	if (bind_no_port)
		SAFE_SETSOCKOPT_INT(sd, SOL_IP, IP_BIND_ADDRESS_NO_PORT, 1);
	if (reuse_port)
		SAFE_SETSOCKOPT_INT(sd, SOL_SOCKET, SO_REUSEPORT, 1);

	SAFE_BIND(sd, local_addrinfo->ai_addr, local_addrinfo->ai_addrlen);

	if (bind_no_port && proto_type != TYPE_SCTP) {
		int port = TST_GETSOCKPORT(sd);

		if (port)
			tst_brk(TFAIL, "port not zero after bind(): %d", port);
	}
}

static int client_connect_send(const char *msg, int size)
{
	int cfd = SAFE_SOCKET(family, sock_type, protocol);

	init_socket_opts(cfd);

	if (fastopen_api) {
		/* Replaces connect() + send()/write() */
		SAFE_SENDTO(1, cfd, msg, size, send_flags | MSG_FASTOPEN,
			remote_addrinfo->ai_addr, remote_addrinfo->ai_addrlen);
	} else {
		bind_before_connect(cfd);
		/* old TCP API */
		SAFE_CONNECT(cfd, remote_addrinfo->ai_addr,
			     remote_addrinfo->ai_addrlen);
		SAFE_SEND(1, cfd, msg, size, send_flags);
	}
	return cfd;
}

union net_size_field {
	char bytes[2];
	uint16_t value;
};

static void make_client_request(char client_msg[], int *cln_len, int *srv_len,
				unsigned int *seed)
{
	if (max_rand_msg_len)
		*cln_len = *srv_len = min_msg_len + rand_r(seed) % max_rand_msg_len;

	memset(client_msg, client_byte, *cln_len);
	client_msg[0] = start_byte;

	/* set size for reply */
	union net_size_field net_size;

	net_size.value = htons(*srv_len);
	client_msg[1] = net_size.bytes[0];
	client_msg[2] = net_size.bytes[1];

	client_msg[*cln_len - 1] = end_byte;
}

void *client_fn(void *id)
{
	int cln_len = init_cln_msg_len,
	    srv_len = init_srv_msg_len;
	struct sock_info inf;
	char buf[max_msg_len];
	char client_msg[max_msg_len];
	int i = 0;
	intptr_t err = 0;
	unsigned int seed = init_seed ^ (intptr_t)id;

	inf.raddr_len = sizeof(inf.raddr);
	inf.etime_cnt = 0;
	inf.eshutdown_cnt = 0;
	inf.timeout = wait_timeout;
	inf.pmtu_err_cnt = 0;

	make_client_request(client_msg, &cln_len, &srv_len, &seed);

	/* connect & send requests */
	inf.fd = client_connect_send(client_msg, cln_len);
	if (inf.fd == -1) {
		err = errno;
		goto out;
	}

	if (client_recv(buf, srv_len, &inf)) {
		err = errno;
		goto out;
	}

	for (i = 1; i < client_max_requests; ++i) {
		if (inf.fd == -1) {
			inf.fd = client_connect_send(client_msg, cln_len);
			if (inf.fd == -1) {
				err = errno;
				goto out;
			}

			if (client_recv(buf, srv_len, &inf)) {
				err = errno;
				break;
			}
			continue;
		}

		if (max_rand_msg_len)
			make_client_request(client_msg, &cln_len, &srv_len, &seed);

		SAFE_SEND(1, inf.fd, client_msg, cln_len, send_flags);

		if (client_recv(buf, srv_len, &inf)) {
			err = errno;
			break;
		}
	}

	if (inf.fd != -1)
		SAFE_CLOSE(inf.fd);

out:
	if (i != client_max_requests)
		tst_res(TWARN, "client exit on '%d' request", i);

	return (void *) err;
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
		tst_brk(TBROK, "Unexpected num of clients '%d'",
			clients_num);
	}

	thread_ids = SAFE_MALLOC(sizeof(pthread_t) * clients_num);

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = sock_type;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if (source_addr)
		SAFE_GETADDRINFO(source_addr, NULL, &hints, &local_addrinfo);
	SAFE_GETADDRINFO(server_addr, tcp_port, &hints, &remote_addrinfo);

	tst_res(TINFO, "Running the test over IPv%s",
		(remote_addrinfo->ai_family == AF_INET6) ? "6" : "4");

	family = remote_addrinfo->ai_family;

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv_client_start);
	intptr_t i;
	for (i = 0; i < clients_num; ++i)
		SAFE_PTHREAD_CREATE(&thread_ids[i], &attr, client_fn, (void *)i);
}

static void client_run(void)
{
	void *res = NULL;
	long clnt_time = 0;
	int i;
	for (i = 0; i < clients_num; ++i) {
		pthread_join(thread_ids[i], &res);
		if (res) {
			tst_brk(TBROK, "client[%d] failed: %s",
				i, strerror((intptr_t)res));
		}
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv_client_end);
	clnt_time = (tv_client_end.tv_sec - tv_client_start.tv_sec) * 1000 +
		(tv_client_end.tv_nsec - tv_client_start.tv_nsec) / 1000000;

	tst_res(TINFO, "total time '%ld' ms", clnt_time);

	char client_msg[min_msg_len];
	int msg_len = min_msg_len;

	max_rand_msg_len = 0;
	make_client_request(client_msg, &msg_len, &msg_len, NULL);
	/* ask server to terminate */
	client_msg[0] = start_fin_byte;
	int cfd = client_connect_send(client_msg, msg_len);
	if (cfd != -1) {
		shutdown(cfd, SHUT_WR);
		SAFE_CLOSE(cfd);
	}
	/* the script tcp_fastopen_run.sh will remove it */
	if (rpath)
		SAFE_FILE_PRINTF(rpath, "%ld", clnt_time);

	tst_res(TPASS, "test completed");
}

static void client_cleanup(void)
{
	free(thread_ids);

	if (remote_addrinfo)
		freeaddrinfo(remote_addrinfo);
}

static void make_server_reply(char *send_msg, int size)
{
	memset(send_msg, server_byte, size - 1);
	send_msg[0] = start_byte;
	send_msg[size - 1] = end_byte;
}

void *server_fn(void *cfd)
{
	int num_requests = 0, offset = 0;
	char send_msg[max_msg_len], end[] = { end_byte };
	int start_send_type = (sock_type == SOCK_DGRAM) ? 1 : 0;
	int send_msg_len, send_type = start_send_type;
	char recv_msg[max_msg_len];
	struct sock_info inf;
	ssize_t recv_len;
	struct iovec iov[2];
	struct msghdr msg;

	inf.fd = (intptr_t) cfd;
	inf.raddr_len = sizeof(inf.raddr);
	inf.timeout = wait_timeout;

	iov[0].iov_base = send_msg;
	iov[1].iov_base = end;
	iov[1].iov_len = 1;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &inf.raddr;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	init_socket_opts(inf.fd);

	while (1) {
		recv_len = sock_recv_poll(recv_msg + offset,
					  max_msg_len - offset, &inf);

		if (recv_len == 0)
			break;

		if (recv_len < 0 || (offset + recv_len) > max_msg_len ||
		   (recv_msg[0] != start_byte &&
		    recv_msg[0] != start_fin_byte)) {
			tst_res(TFAIL, "recv failed, sock '%d'", inf.fd);
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

		send_msg_len = parse_client_request(recv_msg);
		if (send_msg_len < 0) {
			tst_res(TFAIL, "wrong msg size '%d'",
				send_msg_len);
			goto out;
		}
		make_server_reply(send_msg, send_msg_len);

		offset = 0;

		/*
		 * It will tell client that server is going
		 * to close this connection.
		 */
		if (sock_type == SOCK_STREAM &&
		    ++num_requests >= server_max_requests)
			send_msg[0] = start_fin_byte;

		switch (send_type) {
		case 0:
			SAFE_SEND(1, inf.fd, send_msg, send_msg_len,
				  send_flags);
			if (proto_type != TYPE_SCTP)
				++send_type;
			break;
		case 1:
			SAFE_SENDTO(1, inf.fd, send_msg, send_msg_len,
				    send_flags, (struct sockaddr *)&inf.raddr,
				    inf.raddr_len);
			++send_type;
			break;
		default:
			iov[0].iov_len = send_msg_len - 1;
			msg.msg_namelen = inf.raddr_len;
			SAFE_SENDMSG(send_msg_len, inf.fd, &msg, send_flags);
			send_type = start_send_type;
			break;
		}

		if (sock_type == SOCK_STREAM &&
		    num_requests >= server_max_requests) {
			/* max reqs, close socket */
			shutdown(inf.fd, SHUT_WR);
			break;
		}
	}

	SAFE_CLOSE(inf.fd);
	return NULL;

out:
	SAFE_CLOSE(inf.fd);
	tst_brk(TBROK, "Server closed");
	return NULL;
}

static pthread_t server_thread_add(intptr_t client_fd)
{
	pthread_t id;
	SAFE_PTHREAD_CREATE(&id, &attr, server_fn, (void *) client_fd);
	return id;
}

static void server_init(void)
{
	char *src_addr = NULL;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = sock_type;
	hints.ai_flags = AI_PASSIVE;

	if (!source_addr && !tcp_port)
		tcp_port = "0";

	if (source_addr && !strchr(source_addr, ':'))
		SAFE_ASPRINTF(&src_addr, "::ffff:%s", source_addr);
	SAFE_GETADDRINFO(src_addr ? src_addr : source_addr, tcp_port,
		       &hints, &local_addrinfo);
	free(src_addr);

	/* IPv6 socket is also able to access IPv4 protocol stack */
	sfd = SAFE_SOCKET(family, sock_type, protocol);
	SAFE_SETSOCKOPT_INT(sfd, SOL_SOCKET, SO_REUSEADDR, 1);
	if (reuse_port)
		SAFE_SETSOCKOPT_INT(sfd, SOL_SOCKET, SO_REUSEPORT, 1);

	tst_res(TINFO, "assigning a name to the server socket...");
	SAFE_BIND(sfd, local_addrinfo->ai_addr, local_addrinfo->ai_addrlen);

	freeaddrinfo(local_addrinfo);

	int port = TST_GETSOCKPORT(sfd);

	tst_res(TINFO, "bind to port %d", port);
	if (server_bg) {
		SAFE_CHDIR(server_bg);
		SAFE_FILE_PRINTF(port_path, "%d", port);
	}

	if (sock_type == SOCK_DGRAM)
		return;

	init_socket_opts(sfd);

	if (fastopen_api || fastopen_sapi) {
		SAFE_SETSOCKOPT_INT(sfd, IPPROTO_TCP, TCP_FASTOPEN,
			tfo_queue_size);
	}

	if (zcopy)
		SAFE_SETSOCKOPT_INT(sfd, SOL_SOCKET, SO_ZEROCOPY, 1);

	SAFE_LISTEN(sfd, max_queue_len);

	tst_res(TINFO, "Listen on the socket '%d'", sfd);
}

static void server_cleanup(void)
{
	SAFE_CLOSE(sfd);
}

static void move_to_background(void)
{
	if (SAFE_FORK()) {
		TST_CHECKPOINT_WAIT(0);
		exit(0);
	}

	SAFE_SETSID();

	TST_CHECKPOINT_WAKE(0);

	close(STDIN_FILENO);
	SAFE_OPEN("/dev/null", O_RDONLY);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	int fd = SAFE_OPEN(log_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);

	SAFE_DUP(fd);
}

static void server_run_udp(void)
{
	if (server_bg)
		move_to_background();

	pthread_t p_id = server_thread_add(sfd);

	SAFE_PTHREAD_JOIN(p_id, NULL);
}

static void server_run(void)
{
	if (server_bg)
		move_to_background();

	/* IPv4 source address will be mapped to IPv6 address */
	struct sockaddr_in6 addr6;
	socklen_t addr_size = sizeof(addr6);

	/*
	 * detaching threads allow to reclaim thread's resources
	 * once a thread finishes its work.
	 */
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
		tst_brk(TBROK | TERRNO, "setdetachstate failed");

	while (1) {
		int client_fd = accept(sfd, (struct sockaddr *)&addr6,
			&addr_size);

		if (client_fd == -1)
			tst_brk(TBROK, "Can't create client socket");

		server_thread_add(client_fd);
	}
}

static void require_root(const char *file)
{
	if (!geteuid())
		return;
	tst_brk(TCONF, "Test needs to be run as root to change %s", file);
}

static void check_tfo_value(void)
{
	/* Check if we can write to tcp_fastopen knob. We might be
	 * inside netns and either have read-only permission or
	 * doesn't have the knob at all.
	 */
	if (access(tfo_cfg, W_OK) < 0) {
		/* TODO check /proc/self/ns/ or TST_USE_NETNS env var */
		tst_res(TINFO, "can't read %s, assume server runs in netns",
			tfo_cfg);
		return;
	}

	SAFE_FILE_SCANF(tfo_cfg, "%d", &tfo_cfg_value);
	tst_res(TINFO, "'%s' is %d", tfo_cfg, tfo_cfg_value);

	/* The check can be the first in this function but set here
	 * to allow to print information about the currently set config
	 */
	if (tfo_value < 0)
		return;

	if (tfo_cfg_value == tfo_value)
		return;

	require_root(tfo_cfg);

	tst_res(TINFO, "set '%s' to '%d'", tfo_cfg, tfo_value);

	SAFE_FILE_PRINTF(tfo_cfg, "%d", tfo_value);
	tfo_cfg_changed = 1;
}

static void check_tw_reuse(void)
{
	if (access(tcp_tw_reuse, W_OK) < 0)
		return;

	int reuse_value = 0;

	SAFE_FILE_SCANF(tcp_tw_reuse, "%d", &reuse_value);
	if (reuse_value) {
		tst_res(TINFO, "tcp_tw_reuse is already set");
		return;
	}

	require_root(tfo_cfg);

	SAFE_FILE_PRINTF(tcp_tw_reuse, "1");
	tw_reuse_changed = 1;
	tst_res(TINFO, "set '%s' to '1'", tcp_tw_reuse);
}

static void set_protocol_type(void)
{
	if (!type || !strcmp(type, "tcp"))
		proto_type = TYPE_TCP;
	else if (!strcmp(type, "udp"))
		proto_type = TYPE_UDP;
	else if (!strcmp(type, "udp_lite"))
		proto_type = TYPE_UDP_LITE;
	else if (!strcmp(type, "dccp"))
		proto_type = TYPE_DCCP;
	else if (!strcmp(type, "sctp"))
		proto_type = TYPE_SCTP;
	else
		tst_brk(TBROK, "Invalid proto_type: '%s'", type);
}

static void setup(void)
{
	if (tst_parse_int(aarg, &clients_num, 1, INT_MAX))
		tst_brk(TBROK, "Invalid client number '%s'", aarg);
	if (tst_parse_int(rarg, &client_max_requests, 1, INT_MAX))
		tst_brk(TBROK, "Invalid client max requests '%s'", rarg);
	if (tst_parse_int(Rarg, &server_max_requests, 1, INT_MAX))
		tst_brk(TBROK, "Invalid server max requests '%s'", Rarg);
	if (tst_parse_int(narg, &init_cln_msg_len, min_msg_len, max_msg_len))
		tst_brk(TBROK, "Invalid client msg size '%s'", narg);
	if (tst_parse_int(Narg, &init_srv_msg_len, min_msg_len, max_msg_len))
		tst_brk(TBROK, "Invalid server msg size '%s'", Narg);
	if (tst_parse_int(qarg, &tfo_queue_size, 1, INT_MAX))
		tst_brk(TBROK, "Invalid TFO queue size '%s'", qarg);
	if (tst_parse_int(Targ, &wait_timeout, 0, INT_MAX))
		tst_brk(TBROK, "Invalid wait timeout '%s'", Targ);
	if (tst_parse_int(barg, &busy_poll, 0, INT_MAX))
		tst_brk(TBROK, "Invalid busy poll timeout'%s'", barg);
	if (tst_parse_int(targ, &tfo_value, 0, INT_MAX))
		tst_brk(TBROK, "Invalid net.ipv4.tcp_fastopen '%s'", targ);
	if (tst_parse_int(Aarg, &max_rand_msg_len, 10, max_msg_len))
		tst_brk(TBROK, "Invalid max random payload size '%s'", Aarg);

	if (!server_addr)
		server_addr = "localhost";

	if (max_rand_msg_len) {
		max_rand_msg_len -= min_msg_len;
		init_seed = max_rand_msg_len ^ client_max_requests;
		srand(init_seed); /* in case rand_r() is missing */
		tst_res(TINFO, "rand start seed 0x%x", init_seed);
	}

	/* if client_num is not set, use num of processors */
	if (!clients_num)
		clients_num = sysconf(_SC_NPROCESSORS_ONLN);

	if (busy_poll >= 0 && tst_kvercmp(3, 11, 0) < 0)
		tst_brk(TCONF, "Test must be run with kernel 3.11 or newer");

	set_protocol_type();

	if (client_mode) {
		if (source_addr && tst_kvercmp(4, 2, 0) >= 0) {
			bind_no_port = 1;
			tst_res(TINFO, "IP_BIND_ADDRESS_NO_PORT is used");
		}
		tst_res(TINFO, "connection: addr '%s', port '%s'",
			server_addr, tcp_port);
		tst_res(TINFO, "client max req: %d", client_max_requests);
		tst_res(TINFO, "clients num: %d", clients_num);
		if (max_rand_msg_len) {
			tst_res(TINFO, "random msg size [%d %d]",
				min_msg_len, max_rand_msg_len);
		} else {
			tst_res(TINFO, "client msg size: %d", init_cln_msg_len);
			tst_res(TINFO, "server msg size: %d", init_srv_msg_len);
		}
		net.init	= client_init;
		net.run		= client_run;
		net.cleanup	= client_cleanup;

		switch (proto_type) {
		case TYPE_TCP:
			check_tw_reuse();
			break;
		case TYPE_DCCP:
		case TYPE_UDP:
		case TYPE_UDP_LITE:
			if (max_etime_cnt >= client_max_requests)
				max_etime_cnt = client_max_requests - 1;
			tst_res(TINFO, "maximum allowed timeout errors %d", max_etime_cnt);
			wait_timeout = 100;
		}
	} else {
		tst_res(TINFO, "max requests '%d'",
			server_max_requests);
		net.init	= server_init;
		switch (proto_type) {
		case TYPE_TCP:
		case TYPE_DCCP:
		case TYPE_SCTP:
			net.run		= server_run;
			net.cleanup	= server_cleanup;
		break;
		case TYPE_UDP:
		case TYPE_UDP_LITE:
			net.run		= server_run_udp;
			net.cleanup	= NULL;
		break;
		}
	}

	if (zcopy)
		send_flags |= MSG_ZEROCOPY;

	switch (proto_type) {
	case TYPE_TCP:
		tst_res(TINFO, "TCP %s is using %s TCP API.",
			(client_mode) ? "client" : "server",
			(fastopen_api) ? "Fastopen" : "old");
		check_tfo_value();
	break;
	case TYPE_UDP:
		tst_res(TINFO, "using UDP");
		fastopen_api = fastopen_sapi = NULL;
		sock_type = SOCK_DGRAM;
	break;
	case TYPE_UDP_LITE:
		tst_res(TINFO, "using UDP Lite");
		fastopen_api = fastopen_sapi = NULL;
		sock_type = SOCK_DGRAM;
		protocol = IPPROTO_UDPLITE;
	break;
	case TYPE_DCCP: {
		/* dccp* modules can be blacklisted, load them manually */
		const char * const argv[] = {"modprobe", "dccp_ipv6", NULL};

		if (tst_cmd(argv, NULL, NULL, TST_CMD_PASS_RETVAL))
			tst_brk(TCONF, "Failed to load dccp_ipv6 module");

		tst_res(TINFO, "DCCP %s", (client_mode) ? "client" : "server");
		fastopen_api = fastopen_sapi = NULL;
		sock_type = SOCK_DCCP;
		protocol = IPPROTO_DCCP;
		service_code = htonl(service_code);
	} break;
	case TYPE_SCTP:
		tst_res(TINFO, "SCTP %s", (client_mode) ? "client" : "server");
		fastopen_api = fastopen_sapi = NULL;
		protocol = IPPROTO_SCTP;
	break;
	}

	if ((errno = pthread_attr_init(&attr)))
		tst_brk(TBROK | TERRNO, "pthread_attr_init failed");

	if ((errno = pthread_attr_setstacksize(&attr, 256*1024)))
		tst_brk(TBROK | TERRNO, "pthread_attr_setstacksize(256*1024) failed");

	net.init();
}

static void do_test(void)
{
	net.run();
}

static struct tst_test test = {
	.test_all = do_test,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.options = (struct tst_option[]) {
		{"f", &fastopen_api, "Use TFO API, default is old API"},
		{"F", &fastopen_sapi, "TCP_FASTOPEN_CONNECT socket option and standard API"},
		{"t:", &targ, "Set tcp_fastopen value"},
		{"S:", &source_addr, "Source address to bind"},
		{"g:", &tcp_port, "Server port"},
		{"b:", &barg, "Low latency busy poll timeout"},
		{"T:", &type, "Tcp (default), udp, udp_lite, dccp, sctp"},
		{"z", &zcopy, "Enable SO_ZEROCOPY"},
		{"P:", &reuse_port, "Enable SO_REUSEPORT"},
		{"d:", &dev, "Bind to device x"},

		{"H:", &server_addr, "Server name or IP address"},
		{"l", &client_mode, "Become client, default is server"},
		{"a:", &aarg, "Number of clients running in parallel"},
		{"r:", &rarg, "Number of client requests"},
		{"n:", &narg, "Client message size"},
		{"N:", &Narg, "Server message size"},
		{"m:", &Targ, "Receive timeout in milliseconds (not used by UDP/DCCP client)"},
		{"d:", &rpath, "Path to file where result is saved"},
		{"A:", &Aarg, "Max payload length (generated randomly)"},

		{"R:", &Rarg, "Server requests after which conn.closed"},
		{"q:", &qarg, "TFO queue"},
		{"B:", &server_bg, "Run in background, arg is the process directory"},
		{}
	},
	.max_runtime = 300,
	.needs_checkpoints = 1,
};
