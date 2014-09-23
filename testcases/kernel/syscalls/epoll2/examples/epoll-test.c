/*
 *  EpollTest by Davide Libenzi ( Epoll functionality tester )
 *  Copyright (C) 2003  Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sched.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <syslog.h>
#include <glob.h>
#include <semaphore.h>

/*
 * You need the Portable Coroutine Library (PCL) to build this source.
 * You can find a copy of PCL source code at :
 *
 *             http://www.xmailserver.org/libpcl.html
 */
#include <pcl.h>

#include "epoll.h"
#include "dbllist.h"

#define CO_STD_STACK_SIZE		(2 * 4096)
#define STD_SCHED_TIMEOUT		1000
/* you might need to increase "net.ipv4.tcp_max_syn_backlog" to use this value */
#define STD_LISTEN_SIZE			2048
#define DATA_BUFFER_SIZE		2048
#define MIN_AHEAD_SPACE			(DATA_BUFFER_SIZE / 12)
#define STD_MESSAGE_SIZE		128
#define STD_SERVER_PORT			8080
#define MAX_DEFAULT_FDS			20000

struct eph_conn {
	struct list_head lnk;
	int sfd;
	unsigned int events, revents;
	coroutine_t co;
	int nbytes, rindex;
	char buffer[DATA_BUFFER_SIZE];
};

static int kdpfd;
static struct list_head close_list;
static struct epoll_event *events;
static int maxfds, numfds = 0;
static int chash_size;
static struct list_head *chash;
static int msgsize = STD_MESSAGE_SIZE, port = STD_SERVER_PORT,
    maxsfd = MAX_DEFAULT_FDS, stksize = CO_STD_STACK_SIZE;
struct sockaddr_in saddr;
static volatile unsigned long httpresp = 0;
static int nreqsess = 1;
static char httpreq[512] = "";

int eph_socket(int domain, int type, int protocol)
{
	int sfd = socket(domain, type, protocol), flags = 1;

	if (sfd == -1)
		return -1;
	if (ioctl(sfd, FIONBIO, &flags) &&
	    ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
	     fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0)) {
		close(sfd);
		return -1;
	}
	return sfd;
}

int eph_close(int sfd)
{
	close(sfd);
	return 0;
}

static int eph_new_conn(int sfd, void *func)
{
	struct eph_conn *conn = malloc(sizeof(struct eph_conn));
	struct epoll_event ev;

	if (!conn)
		return -1;

	memset(conn, 0, sizeof(*conn));
	DBL_INIT_LIST_HEAD(&conn->lnk);
	conn->sfd = sfd;
	conn->events = 0;
	conn->revents = 0;
	conn->nbytes = conn->rindex = 0;
	if (!(conn->co = co_create(func, conn, NULL, stksize))) {
		free(conn);
		return -1;
	}

	DBL_LIST_ADDT(&conn->lnk, &chash[sfd % chash_size]);

	ev.events = 0;
	ev.data.ptr = conn;
	if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, sfd, &ev) < 0) {
		fprintf(stderr, "epoll set insertion error: fd=%d\n", sfd);

		DBL_LIST_DEL(&conn->lnk);
		co_delete(conn->co);
		free(conn);
		return -1;
	}

	++numfds;

	co_call(conn->co);

	return 0;
}

static void eph_exit_conn(struct eph_conn *conn)
{
	struct epoll_event ev;

	if (epoll_ctl(kdpfd, EPOLL_CTL_DEL, conn->sfd, &ev) < 0) {
		fprintf(stderr, "epoll set deletion error: fd=%d\n", conn->sfd);

	}

	DBL_LIST_DEL(&conn->lnk);
	DBL_LIST_ADDT(&conn->lnk, &close_list);

	eph_close(conn->sfd);
	conn->sfd = -1;

	--numfds;

	co_exit();
}

static void eph_free_conns(void)
{
	struct eph_conn *conn;

	while (!DBL_LIST_EMTPY(&close_list)) {
		conn = DBL_LIST_ENTRY(close_list.pNext, struct eph_conn, lnk);

		DBL_LIST_DEL(&conn->lnk);
		free(conn);
	}
}

static int eph_mod_conn(struct eph_conn *conn, unsigned int events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.ptr = conn;
	if (epoll_ctl(kdpfd, EPOLL_CTL_MOD, conn->sfd, &ev) < 0) {
		fprintf(stderr, "epoll set modify error: fd=%d\n", conn->sfd);
		return -1;
	}
	return 0;
}

int eph_connect(struct eph_conn *conn, const struct sockaddr *serv_addr,
		socklen_t addrlen)
{

	if (connect(conn->sfd, serv_addr, addrlen) == -1) {
		if (errno != EWOULDBLOCK && errno != EINPROGRESS)
			return -1;
		if (!(conn->events & EPOLLOUT)) {
			conn->events = EPOLLOUT | EPOLLERR | EPOLLHUP;
			if (eph_mod_conn(conn, conn->events) < 0)
				return -1;
		}
		co_resume();
		if (conn->revents & (EPOLLERR | EPOLLHUP))
			return -1;
	}
	return 0;
}

int eph_read(struct eph_conn *conn, char *buf, int nbyte)
{
	int n;

	while ((n = read(conn->sfd, buf, nbyte)) < 0) {
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			return -1;
		if (!(conn->events & EPOLLIN)) {
			conn->events = EPOLLIN | EPOLLERR | EPOLLHUP;
			if (eph_mod_conn(conn, conn->events) < 0)
				return -1;
		}
		co_resume();
	}
	return n;
}

int eph_write(struct eph_conn *conn, char const *buf, int nbyte)
{
	int n;

	while ((n = write(conn->sfd, buf, nbyte)) < 0) {
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			return -1;
		if (!(conn->events & EPOLLOUT)) {
			conn->events = EPOLLOUT | EPOLLERR | EPOLLHUP;
			if (eph_mod_conn(conn, conn->events) < 0)
				return -1;
		}
		co_resume();
	}
	return n;
}

int eph_accept(struct eph_conn *conn, struct sockaddr *addr, int *addrlen)
{
	int sfd, flags = 1;

	while ((sfd = accept(conn->sfd, addr, (socklen_t *) addrlen)) < 0) {
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			return -1;
		if (!(conn->events & EPOLLIN)) {
			conn->events = EPOLLIN | EPOLLERR | EPOLLHUP;
			if (eph_mod_conn(conn, conn->events) < 0)
				return -1;
		}
		co_resume();
	}
	if (ioctl(sfd, FIONBIO, &flags) &&
	    ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
	     fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0)) {
		close(sfd);
		return -1;
	}
	return sfd;
}

static int eph_create_conn(int domain, int type, int protocol, void *func)
{
	int sfd = eph_socket(domain, type, protocol);

	return sfd != -1 ? eph_new_conn(sfd, func) : -1;
}

static int eph_read_data(struct eph_conn *conn)
{
	int nbytes;

	if (conn->rindex && conn->rindex < conn->nbytes) {
		memmove(conn->buffer, conn->buffer + conn->rindex,
			conn->nbytes - conn->rindex);
		conn->nbytes -= conn->rindex;
	} else
		conn->nbytes = 0;

	conn->rindex = 0;

	if ((nbytes = eph_read(conn, conn->buffer + conn->nbytes,
			       sizeof(conn->buffer) - conn->nbytes)) <= 0)
		return -1;

	conn->nbytes += nbytes;

	return 0;
}

static int eph_write_data(struct eph_conn *conn, char const *buf, int nbyte)
{
	int wbytes, wcurr;

	for (wbytes = 0; wbytes < nbyte;) {
		if ((wcurr = eph_write(conn, buf + wbytes, nbyte - wbytes)) < 0)
			break;
		wbytes += wcurr;
	}

	return wbytes;
}

static char *eph_read_line(struct eph_conn *conn)
{
	char *nline, *line;

	for (;;) {
		if (conn->nbytes > conn->rindex) {
			if ((nline = memchr(conn->buffer + conn->rindex, '\n',
					    conn->nbytes - conn->rindex))) {
				line = conn->buffer + conn->rindex;
				conn->rindex += (nline - line) + 1;
				for (; nline > line && nline[-1] == '\r';
				     nline--) ;
				*nline = '\0';
				return line;
			}
		}
		if (eph_read_data(conn) < 0)
			break;
	}
	return NULL;
}

static int eph_parse_request(struct eph_conn *conn)
{
	char *line;

	if (!(line = eph_read_line(conn)))
		return -1;

	for (;;) {
		if (!(line = eph_read_line(conn)))
			return -1;

		if (*line == '\0')
			break;
	}

	return 0;
}

static int eph_send_response(struct eph_conn *conn)
{
	static int resplen = -1;
	static char *resp = NULL;

	if (resp == NULL) {
		msgsize = ((msgsize + 63) / 64) * 64;

		resp = malloc(msgsize + 256);

		sprintf(resp,
			"HTTP/1.1 200 OK\r\n"
			"Server: dp server\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: %d\r\n" "\r\n", msgsize);

		while (msgsize > 0) {
			strcat(resp,
			       "01234567890123\r\n"
			       "01234567890123\r\n"
			       "01234567890123\r\n" "01234567890123\r\n");
			msgsize -= 64;
		}

		resplen = strlen(resp);
	}

	if (eph_write_data(conn, resp, resplen) != resplen)
		return -1;

	return 0;
}

static void *eph_httpd(void *data)
{
	struct eph_conn *conn = (struct eph_conn *)data;

	while (eph_parse_request(conn) == 0) {
		eph_send_response(conn);

	}

	eph_exit_conn(conn);
	return data;
}

static void *eph_acceptor(void *data)
{
	struct eph_conn *conn = (struct eph_conn *)data;
	struct sockaddr_in addr;
	int sfd, addrlen = sizeof(addr);

	while ((sfd =
		eph_accept(conn, (struct sockaddr *)&addr, &addrlen)) != -1) {
		if (eph_new_conn(sfd, eph_httpd) < 0) {
			eph_close(sfd);

		}
	}
	eph_exit_conn(conn);
	return data;
}

static struct eph_conn *eph_find(int sfd)
{
	struct list_head *head = &chash[sfd % chash_size], *lnk;
	struct eph_conn *conn;

	DBL_LIST_FOR_EACH(lnk, head) {
		conn = DBL_LIST_ENTRY(lnk, struct eph_conn, lnk);

		if (conn->sfd == sfd)
			return conn;
	}
	return NULL;
}

static int eph_runqueue(void)
{
	int i;
	struct list_head *head, *lnk;
	struct eph_conn *conn;

	for (i = 0; i < chash_size; i++) {
		head = &chash[i];
		for (lnk = head->pNext; lnk != head;) {
			conn = DBL_LIST_ENTRY(lnk, struct eph_conn, lnk);

			lnk = lnk->pNext;
			co_call(conn->co);
		}
	}
	return 0;
}

unsigned long long eph_mstics(void)
{

	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (0);

	return (1000 * (unsigned long long)tv.tv_sec +
		(unsigned long long)tv.tv_usec / 1000);

}

int eph_init(void)
{
	int i;

	if (!
	    (events = malloc(maxsfd * sizeof(struct epoll_event)))) {
		perror("malloc()");
		return -1;
	}

	if ((kdpfd = epoll_create(maxsfd)) < 0) {
		perror("epoll_create");
		return -1;
	}

	if (!
	    (chash = malloc(maxsfd * sizeof(struct list_head)))) {
		perror("malloc()");
		free(events);
		close(kdpfd);
		return -1;
	}

	maxfds = maxsfd;
	chash_size = maxfds;
	for (i = 0; i < maxfds; i++)
		DBL_INIT_LIST_HEAD(&chash[i]);

	DBL_INIT_LIST_HEAD(&close_list);

	return 0;
}

int eph_cleanup(void)
{

	free(events);
	free(chash);
	close(kdpfd);
	return 0;
}

static int eph_scheduler(int loop, unsigned int timeout)
{
	int i, nfds;
	struct eph_conn *conn;
	struct epoll_event *cevents;

	do {
		nfds = epoll_wait(kdpfd, events, maxfds, timeout);

		for (i = 0, cevents = events; i < nfds; i++, cevents++) {
			conn = cevents->data.ptr;
			if (conn->sfd != -1) {
				conn->revents = cevents->events;

				if (conn->revents & conn->events)
					co_call(conn->co);
			}
		}
#if 0
		if (nfds <= 0)
			eph_runqueue();
#endif
		eph_free_conns();
	} while (loop);

	return 0;
}

#if defined(DPHTTPD)

void eph_usage(char const *prgname)
{

	fprintf(stderr,
		"use: %s [--msgsize nbytes (%d)] [--port nbr (%d)] [--maxfds nfds (%d)]\n\t[--stksize bytes (%d)]\n",
		prgname, msgsize, port, maxsfd, stksize);

}

int main(int argc, char *argv[])
{
	int i, sfd, flags = 1;
	struct linger ling = { 0, 0 };
	struct sockaddr_in addr;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--msgsize") == 0) {
			if (++i < argc)
				msgsize = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--port") == 0) {
			if (++i < argc)
				port = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--maxfds") == 0) {
			if (++i < argc)
				maxsfd = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--stksize") == 0) {
			if (++i < argc)
				stksize = atoi(argv[i]);
			continue;
		}

		eph_usage(argv[0]);
		return 1;
	}

	if (eph_init() == -1) {

		return 2;
	}

	if ((sfd = eph_socket(AF_INET, SOCK_STREAM, 0)) == -1) {

		eph_cleanup();
		return 3;
	}

	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
	setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));
	setsockopt(sfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {

		eph_close(sfd);
		eph_cleanup();
		return 4;
	}

	listen(sfd, STD_LISTEN_SIZE);

	if (eph_new_conn(sfd, (void *)eph_acceptor) == -1) {

		eph_close(sfd);
		eph_cleanup();
		return 5;
	}

	do {
		eph_scheduler(0, STD_SCHED_TIMEOUT);
	} while (numfds);

	eph_cleanup();
	return 0;
}

#endif /* #if defined(DPHTTPD) */

#if defined(HTTP_BLASTER)

static void *eph_http_session(void *data)
{
	int i, rlen = strlen(httpreq), ava;
	struct eph_conn *conn = (struct eph_conn *)data;

	if (eph_connect(conn, (struct sockaddr *)&saddr, sizeof(saddr)) == 0) {
		for (i = 0; i < nreqsess; i++) {
			if (eph_write_data(conn, httpreq, rlen) == rlen) {
				static char const *clent = "Content-Length:";
				int length = -1, clens = strlen(clent);
				char *line;
				static char buf[2048];

				while ((line = eph_read_line(conn))) {
					if (*line == '\0')
						break;
					if (strncasecmp(line, clent, clens) ==
					    0) {
						for (line += clens;
						     *line == ' '; line++) ;
						length = atoi(line);
					}
				}
				if (length < 0)
					goto sess_out;
				if ((ava = conn->nbytes - conn->rindex) > 0) {
					if (ava > length)
						ava = length;
					length -= ava;
					conn->rindex += ava;
				}
				++httpresp;
				while (length > 0) {
					int rsiz =
					    length >
					    sizeof(buf) ? sizeof(buf) : length;

					if ((rsiz =
					     eph_read(conn, buf, rsiz)) <= 0)
						goto sess_out;
					length -= rsiz;
				}
			} else
				goto sess_out;
		}
	}
sess_out:
	eph_exit_conn(conn);
	return data;
}

void eph_usage(char const *prgname)
{

	fprintf(stderr,
		"use: %s  --server serv	 --port nprt  --numconns ncon  [--nreq nreq (%d)]\n"
		"[--maxconns ncon] [--url url ('/')] [--stksize bytes (%d)]\n",
		prgname, nreqsess, stksize);

}

int main(int argc, char *argv[])
{
	int i, nconns = 0, totconns = 0, maxconns = 0;
	unsigned long resplast;
	unsigned long long tinit, tlast, tcurr;
	struct hostent *he;
	char const *server = NULL, *url = "/";
	struct in_addr inadr;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--server") == 0) {
			if (++i < argc)
				server = argv[i];
			continue;
		}
		if (strcmp(argv[i], "--port") == 0) {
			if (++i < argc)
				port = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--maxconns") == 0) {
			if (++i < argc)
				maxconns = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--numconns") == 0) {
			if (++i < argc) {
				nconns = atoi(argv[i]);
				if (nconns > maxsfd)
					maxsfd = nconns + nconns >> 1 + 1;
			}
			continue;
		}
		if (strcmp(argv[i], "--nreq") == 0) {
			if (++i < argc)
				nreqsess = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--url") == 0) {
			if (++i < argc)
				url = argv[i];
			continue;
		}
		if (strcmp(argv[i], "--stksize") == 0) {
			if (++i < argc)
				stksize = atoi(argv[i]);
			continue;
		}

		eph_usage(argv[0]);
		return 1;
	}

	if (!server || !nconns) {
		eph_usage(argv[0]);
		return 2;
	}

	sprintf(httpreq,
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n" "Connection: keepalive\r\n" "\r\n", url, server);

	if (inet_aton(server, &inadr) == 0) {
		if ((he = gethostbyname(server)) == NULL) {
			fprintf(stderr, "unable to resolve: %s\n", server);
			return (-1);
		}

		memcpy(&inadr.s_addr, he->h_addr_list[0], he->h_length);
	}
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	memcpy(&saddr.sin_addr, &inadr.s_addr, 4);

	if (eph_init() == -1) {

		return 2;
	}

	resplast = 0;
	tinit = tlast = eph_mstics();

	for (; numfds || (!maxconns || totconns < maxconns);) {
		int nfds = numfds, errs = 0, diffconns = nconns - numfds;

		while (numfds < nconns && (!maxconns || totconns < maxconns)) {
			eph_create_conn(AF_INET, SOCK_STREAM, 0,
					eph_http_session);
			if (nfds == numfds) {
				++errs;
				if (errs > 32) {
					fprintf(stderr,
						"unable to connect: server=%s errors=%d\n",
						server, errs);
					goto main_exit;
				}
			} else
				++totconns;
			nfds = numfds;
		}

		eph_scheduler(0, STD_SCHED_TIMEOUT);

		tcurr = eph_mstics();
		if ((tcurr - tlast) >= 1000) {
			printf
			    ("rate = %lu  avg = %lu  totconns = %d  diff = %d  resp = %ld  nfds = %d\n",
			     (unsigned long)((1000 * (httpresp - resplast)) /
					     (tcurr - tlast)),
			     (unsigned long)((1000 * httpresp) /
					     (tcurr - tinit)), totconns,
			     diffconns, httpresp, numfds);

			tlast = tcurr;
			resplast = httpresp;
		}
	}

main_exit:
	eph_cleanup();
	return 0;
}

#endif /* #if defined(HTTP_BLASTER) */

#if defined(PIPETESTER)

int eph_createcgi(char **args, void *func)
{
	int fds[2], flags = 1;
	pid_t chpid;

	if (pipe(fds)) {
		perror("pipe");
		return -1;
	}
	chpid = fork();
	if (chpid == -1) {
		perror("fork");
		close(fds[0]), close(fds[1]);
		return -1;
	} else if (chpid == 0) {
		close(fds[0]);
		dup2(fds[1], 1);
		close(fds[1]);
		execvp(args[0], args);
		perror("exec");
		exit(1);
	}
	close(fds[1]);
	if (ioctl(fds[0], FIONBIO, &flags) &&
	    ((flags = fcntl(fds[0], F_GETFL, 0)) < 0 ||
	     fcntl(fds[0], F_SETFL, flags | O_NONBLOCK) < 0)) {
		close(fds[0]);
		return -1;
	}
	fprintf(stdout, "child-run=%d  fd=%d\n", chpid, fds[0]), fflush(stdout);
	return eph_new_conn(fds[0], func);
}

int eph_createpipetest(int size, int tsleep, int ttime, void *func)
{
	int fds[2], flags = 1;
	pid_t chpid;

	if (pipe(fds)) {
		perror("pipe");
		return -1;
	}
	chpid = fork();
	if (chpid == -1) {
		perror("fork");
		close(fds[0]), close(fds[1]);
		return -1;
	} else if (chpid == 0) {
		int i;
		char *buff = malloc(size + 1);
		close(fds[0]);
		dup2(fds[1], 1);
		close(fds[1]);

		srand(getpid() * time(NULL));
		for (i = 0; i < (size - 1); i++) {
			if (i && !(i % 64))
				buff[i] = '\n';
			else
				buff[i] = '0' + (rand() % 10);
		}
		buff[i++] = '\n';
		buff[i] = '\0';
		ttime += (ttime * rand()) / RAND_MAX - (ttime >> 1);
		ttime *= 1000;
		while (ttime > 0) {
			usleep(tsleep * 1000);
			fputs(buff, stdout), fflush(stdout);
			ttime -= tsleep;
		}
		free(buff);
		exit(0);
	}
	close(fds[1]);
	if (ioctl(fds[0], FIONBIO, &flags) &&
	    ((flags = fcntl(fds[0], F_GETFL, 0)) < 0 ||
	     fcntl(fds[0], F_SETFL, flags | O_NONBLOCK) < 0)) {
		close(fds[0]);
		return -1;
	}
	fprintf(stdout, "child-run=%d  fd=%d\n", chpid, fds[0]), fflush(stdout);
	return eph_new_conn(fds[0], func);
}

static void *eph_pipe_session(void *data)
{
	struct eph_conn *conn = (struct eph_conn *)data;
	int nbytes, totbytes = 0;
	char buff[257];

	while ((nbytes = eph_read(conn, buff, sizeof(buff))) > 0) {
		fprintf(stdout, "[%p] %d bytes readed\n", conn, nbytes),
		    fflush(stdout);
		totbytes += nbytes;
	}
	fprintf(stdout, "[%p] exit - totbytes=%d\n", conn, totbytes),
	    fflush(stdout);
	eph_exit_conn(conn);
	return data;
}

void eph_sigchld(int sig)
{
	int status;
	pid_t pid;

	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		fprintf(stdout, "child-dead=%d\n", pid), fflush(stdout);
	}
	signal(SIGCHLD, eph_sigchld);
}

void eph_usage(char const *prgname)
{

	fprintf(stderr,
		"use: %s  [--ncgis ncgi]  [--cgi cgi] [--stksize bytes (%d)]\n",
		prgname, stksize);

}

int main(int argc, char *argv[])
{
	int i, ncgis = 8;
	char *cgi = NULL;
	char *args[16];

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--ncgis") == 0) {
			if (++i < argc)
				ncgis = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--cgi") == 0) {
			if (++i < argc)
				cgi = argv[i];
			continue;
		}
		if (strcmp(argv[i], "--stksize") == 0) {
			if (++i < argc)
				stksize = atoi(argv[i]);
			continue;
		}

		eph_usage(argv[0]);
		return 1;
	}

	signal(SIGCHLD, eph_sigchld);
	signal(SIGPIPE, SIG_IGN);

	if (eph_init() == -1) {

		return 2;
	}

	if (cgi) {
		args[0] = cgi;
		args[1] = NULL;

		for (i = 0; i < ncgis; i++)
			eph_createcgi(args, eph_pipe_session);
	} else {
		for (i = 0; i < ncgis; i++)
			eph_createpipetest(256, 250, 8, eph_pipe_session);
	}

	while (numfds > 0)
		eph_scheduler(0, STD_SCHED_TIMEOUT);

	eph_cleanup();
	return 0;
}

#endif /* #if defined(PIPETESTER) */
