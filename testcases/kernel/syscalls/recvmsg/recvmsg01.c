// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2001 Wayne Boyer International Business Machines
 * Copyright (c) Linux Test Project, 2002-2022
 * Copyright (c) 2023 Wei Gao <wegao@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that recvmsg() returns the proper errno for various failure cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

#define MSG "from recvmsg01 server"
#define BUF_SIZE 1024
#define CONTROL_LEN (128 * 1024)

static char recv_buf[BUF_SIZE], cbuf[BUF_SIZE];
static int sock;
static struct sockaddr_in sin1, from;
static struct sockaddr_un sun1;
static struct msghdr msgdat;
static struct cmsghdr *control;
static int controllen;
static struct iovec iov[1];
static int sfd;			/* shared between do_child and start_server */
static int ufd;			/* shared between do_child and start_server */
static pid_t pid;
static char tmpsunpath[BUF_SIZE];

static void setup_all(void);
static void setup_invalid_sock(int);
static void setup_valid_sock(int);
static void setup_valid_msg_control(int);
static void setup_large_msg_control(int);
static void cleanup_all(void);
static void cleanup_invalid_sock(int);
static void cleanup_close_sock(int);
static void cleanup_reset_all(int);
static void do_child(void);
static pid_t start_server(struct sockaddr_in *, struct sockaddr_un *);

static struct tcase {
	int domain;
	int type;
	int protocol;
	struct iovec *iov;
	int iovcnt;
	void *recv_buf;
	int buflen;
	struct msghdr *msg;
	unsigned int flags;
	struct sockaddr *from;
	int fromlen;
	int exp_errno;
	void (*setup)(int n);
	void (*cleanup)(int n);
	char *desc;
} tcases[] = {
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.exp_errno = EBADF,
		.setup = setup_invalid_sock,
		.cleanup = cleanup_invalid_sock,
		.desc = "bad file descriptor",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = (void *)recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.exp_errno = ENOTSOCK,
		.setup = setup_invalid_sock,
		.cleanup = cleanup_invalid_sock,
		.desc = "invalid socket",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = (void *)recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.flags = -1,
		.from = (struct sockaddr *)&from,
		.fromlen = -1,
		.exp_errno = EINVAL,
		.setup = setup_valid_sock,
		.cleanup = cleanup_close_sock,
		.desc = "invalid socket length",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = (void *)-1,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.exp_errno = EFAULT,
		.setup = setup_valid_sock,
		.cleanup = cleanup_close_sock,
		.desc = "invalid recv buffer",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iovcnt = 1,
		.recv_buf = recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.exp_errno = EFAULT,
		.setup = setup_valid_sock,
		.cleanup = cleanup_close_sock,
		.desc = "invalid iovec buffer",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = -1,
		.recv_buf = recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.exp_errno = EMSGSIZE,
		.setup = setup_valid_sock,
		.cleanup = cleanup_close_sock,
		.desc = "invalid iovec count",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.setup = setup_valid_msg_control,
		.cleanup = cleanup_reset_all,
		.desc = "permission reception",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.flags = MSG_OOB,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.exp_errno = EINVAL,
		.setup = setup_valid_sock,
		.cleanup = cleanup_close_sock,
		.desc = "invalid MSG_OOB flag set",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.flags = MSG_ERRQUEUE,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.exp_errno = EAGAIN,
		.setup = setup_valid_sock,
		.cleanup = cleanup_close_sock,
		.desc = "invalid MSG_ERRQUEUE flag set",
	},
	{
		.domain = PF_INET,
		.type = SOCK_STREAM,
		.iov = iov,
		.iovcnt = 1,
		.recv_buf = recv_buf,
		.buflen = sizeof(recv_buf),
		.msg = &msgdat,
		.from = (struct sockaddr *)&from,
		.fromlen = sizeof(from),
		.setup = setup_large_msg_control,
		.cleanup = cleanup_reset_all,
		.desc = "large cmesg length",
	},

};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int ret = tc->exp_errno ? -1 : 0;

	if ((tst_kvercmp(3, 17, 0) < 0)
			&& (tc->flags & MSG_ERRQUEUE)
			&& (tc->type & SOCK_STREAM)) {
		tst_res(TCONF, "MSG_ERRQUEUE requires kernel >= 3.17");
		return;
	}

	setup_all();
	tc->setup(n);

	iov[0].iov_base = tc->recv_buf;
	iov[0].iov_len = tc->buflen;
	msgdat.msg_name = tc->from;
	msgdat.msg_namelen = tc->fromlen;
	msgdat.msg_iov = tc->iov;
	msgdat.msg_iovlen = tc->iovcnt;
	msgdat.msg_control = control;
	msgdat.msg_controllen = controllen;
	msgdat.msg_flags = 0;

	TEST(recvmsg(sock, tc->msg, tc->flags));
	if (TST_RET >= 0)
		TST_RET = 0;

	if (TST_RET != ret) {
		tst_res(TFAIL | TTERRNO, "%s: expected %d, returned %ld",
			tc->desc, ret, TST_RET);
	} else if (TST_ERR != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"%s: expected %s",
			tc->desc, tst_strerrno(tc->exp_errno));
	} else {
		tst_res(TPASS, "%s passed", tc->desc);
	}

	tc->cleanup(n);
	cleanup_all();
}


static void setup_all(void)
{
	int tfd;

	sun1.sun_family = AF_UNIX;

	(void)strcpy(tmpsunpath, "udsockXXXXXX");
	tfd = mkstemp(tmpsunpath);
	SAFE_CLOSE(tfd);
	SAFE_UNLINK(tmpsunpath);
	(void)strcpy(sun1.sun_path, tmpsunpath);
	SAFE_SIGNAL(SIGPIPE, SIG_IGN);
	pid = start_server(&sin1, &sun1);
}

static void cleanup_all(void)
{
	if (pid > 0) {
		(void)kill(pid, SIGKILL);	/* kill server */
		wait(NULL);
	}

	if (tmpsunpath[0] != '\0')
		(void)SAFE_UNLINK(tmpsunpath);
}

static void setup_invalid_sock(int n)
{
	if (tcases[n].exp_errno == EBADF)
		sock = 400;	/* anything not an open file */
	else
		sock = SAFE_OPEN("/dev/null", O_WRONLY);
}

static void cleanup_invalid_sock(int n)
{
	if (tcases[n].exp_errno == EBADF)
		sock = -1;
	else
		SAFE_CLOSE(sock);
}

static void setup_valid_sock(int n)
{
	fd_set rdfds;
	struct timeval timeout;

	sock = SAFE_SOCKET(tcases[n].domain, tcases[n].type, tcases[n].protocol);

	if (tcases[n].type == SOCK_STREAM) {
		if (tcases[n].domain == PF_INET) {
			SAFE_CONNECT(sock, (struct sockaddr *)&sin1, sizeof(sin1));
			/* Wait for something to be readable, else we won't detect EFAULT on recv */
			FD_ZERO(&rdfds);
			FD_SET(sock, &rdfds);
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
			n = select(sock + 1, &rdfds, 0, 0, &timeout);

			if (n != 1 || !FD_ISSET(sock, &rdfds))
				tst_brk(TBROK, "no message ready in %d sec", (int)timeout.tv_sec);

		} else if (tcases[n].domain == PF_UNIX) {
			SAFE_CONNECT(sock, (struct sockaddr *)&sun1, sizeof(sun1));
		}
	}
}

static void setup_valid_msg_control(int n)
{
	setup_valid_sock(n);
	SAFE_SEND(1, sock, "R", 1, 0);
	control = (struct cmsghdr *)cbuf;
	controllen = control->cmsg_len = sizeof(cbuf);
}

static void setup_large_msg_control(int n)
{
	setup_valid_msg_control(n);
	controllen = CONTROL_LEN;
}

static void cleanup_close_sock(int n LTP_ATTRIBUTE_UNUSED)
{
	SAFE_CLOSE(sock);
}

static void cleanup_reset_all(int n LTP_ATTRIBUTE_UNUSED)
{
	SAFE_CLOSE(sock);

	control = 0;
	controllen = 0;
}

pid_t start_server(struct sockaddr_in *ssin, struct sockaddr_un *ssun)
{
	pid_t pid;
	socklen_t slen = sizeof(*ssin);

	ssin->sin_family = AF_INET;
	ssin->sin_port = 0; /* pick random free port */
	ssin->sin_addr.s_addr = INADDR_ANY;

	/* set up inet socket */
	sfd = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	SAFE_BIND(sfd, (struct sockaddr *)ssin, sizeof(*ssin));
	SAFE_LISTEN(sfd, 10);
	SAFE_GETSOCKNAME(sfd, (struct sockaddr *)ssin, &slen);

	/* set up UNIX-domain socket */
	ufd = SAFE_SOCKET(PF_UNIX, SOCK_STREAM, 0);
	SAFE_BIND(ufd, (struct sockaddr *)ssun, sizeof(*ssun));
	SAFE_LISTEN(ufd, 10);

	pid = SAFE_FORK();
	if (!pid) {
		do_child();
		exit(1);
	}

	SAFE_CLOSE(sfd);
	SAFE_CLOSE(ufd);

	return pid;
}

/* for permission test */
static void sender(int fd)
{
	struct msghdr mh = {};
	struct cmsghdr *control;
	char tmpfn[BUF_SIZE] = "";
	char snd_cbuf[BUF_SIZE] = "";
	int tfd;

	(void)strcpy(tmpfn, "smtXXXXXX");
	tfd = mkstemp(tmpfn);
	if (tfd < 0)
		return;

	/* set up cmsghdr */
	control = (struct cmsghdr *)snd_cbuf;
	control->cmsg_len = sizeof(struct cmsghdr) + 4;
	control->cmsg_level = SOL_SOCKET;
	control->cmsg_type = SCM_RIGHTS;
	*(int *)CMSG_DATA(control) = tfd;

	/* set up msghdr */
	iov[0].iov_base = MSG;
	iov[0].iov_len = sizeof(MSG);
	mh.msg_iov = iov;
	mh.msg_iovlen = 1;
	mh.msg_flags = 0;
	mh.msg_control = control;
	mh.msg_controllen = control->cmsg_len;

	/* do it */
	SAFE_SENDMSG(sizeof(MSG), fd, &mh, 0);
	SAFE_CLOSE(tfd);
	(void)SAFE_UNLINK(tmpfn);
}

static void do_child(void)
{
	struct sockaddr_in fsin;
	struct sockaddr_un fsun;
	fd_set afds, rfds;
	int nfds, fd;

	FD_ZERO(&afds);
	FD_SET(sfd, &afds);
	FD_SET(ufd, &afds);

	nfds = MAX(sfd + 1, ufd + 1);

	/* accept connections until killed */
	while (1) {
		socklen_t fromlen;

		memcpy(&rfds, &afds, sizeof(rfds));

		if (select(nfds, &rfds, NULL, NULL,
			   NULL) < 0) {
			if (errno != EINTR) {
				perror("server select");
				exit(1);
			}
			continue;
		}
		if (FD_ISSET(sfd, &rfds)) {
			int newfd;

			fromlen = sizeof(fsin);
			newfd = SAFE_ACCEPT(sfd, (struct sockaddr *)&fsin, &fromlen);
			if (newfd >= 0) {
				FD_SET(newfd, &afds);
				nfds = MAX(nfds, newfd + 1);
				/* send something back */
				SAFE_SEND(1, newfd, "hi", 2, 0);
			}
		}
		if (FD_ISSET(ufd, &rfds)) {
			int newfd;

			fromlen = sizeof(fsun);
			newfd = SAFE_ACCEPT(ufd, (struct sockaddr *)&fsun, &fromlen);
			if (newfd >= 0) {
				FD_SET(newfd, &afds);
				nfds = MAX(nfds, newfd + 1);
			}
		}
		for (fd = 0; fd < nfds; ++fd)
			if (fd != sfd && fd != ufd && FD_ISSET(fd, &rfds)) {
				char rbuf[BUF_SIZE];

				TEST(read(fd, rbuf, sizeof(rbuf)));
				if (TST_RET > 0 && rbuf[0] == 'R')
					sender(fd);
				if (TST_RET == 0 || (TST_RET < 0 && TST_ERR != EINTR)) {
					close(fd);
					FD_CLR(fd, &afds);
				}
			}
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.needs_tmpdir = 1,
};
