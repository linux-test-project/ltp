/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

char my_path[1024];
#define CLEANUP_AND_EXIT do { unlink(my_path); exit(1); } while (0)

void handler(int sig)
{
	CLEANUP_AND_EXIT;
}

int main(int argc, char **argv)
{
	struct sockaddr_un sun;
	char buf[1024];
	int i, s, sunlen, ret, ctrl, *fdptr, fd;
	struct msghdr msg = { 0 };
	struct iovec iov;
	struct cmsghdr *cmsg;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];

	if (argc != 2) {
		fprintf(stderr, "usage:  %s address\n", argv[0]);
		exit(-1);
	}

	for (i = 0; i < 32; i++) {
		signal(i, handler);
	}

	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		exit(-1);
	}

	sun.sun_family = AF_UNIX;
	sunlen = sizeof(struct sockaddr_un);
	strcpy(sun.sun_path, argv[1]);
	sunlen = strlen(sun.sun_path) + 1 + sizeof(short);
	strcpy(my_path, sun.sun_path);
	ret = bind(s, (struct sockaddr*)&sun, sunlen);
	if (ret < 0) {
		perror("bind");
		exit(-1);
	}

	ret = listen(s, 5);
	if (ret < 0) {
		perror("listen");
		CLEANUP_AND_EXIT;
	}

	while (1) {
		sunlen = sizeof(struct sockaddr_un);
		ctrl = accept(s, (struct sockaddr*)&sun, (socklen_t *)&sunlen);
		if (ctrl < 0) {
			perror("accept_secure");
			CLEANUP_AND_EXIT;
		}

		printf("server:  Accepted a connection, receiving message\n");

		buf[0] = 0;
		msg.msg_controllen = 0;
		iov.iov_base = buf;
		iov.iov_len = sizeof(buf);
		msg.msg_name = 0;
		msg.msg_namelen = 0;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = cmsgbuf;
		msg.msg_controllen = sizeof cmsgbuf;
		ret = recvmsg(ctrl, &msg, 0);
		if (ret < 0) {
			perror("recv");
			CLEANUP_AND_EXIT;
		}

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg,cmsg)) {
			if (cmsg->cmsg_level == SOL_SOCKET &&
			    cmsg->cmsg_type == SCM_RIGHTS) {
				fdptr = (int*)CMSG_DATA(cmsg);
				fd = *fdptr;
				printf("server:  Received a descriptor, fd=%d, sending back 0\n", fd);
				buf[0] = 0;
				break;
			}
		}
		if (!cmsg) {
			printf("server:  Received no descriptor, sending back 1\n");
			buf[0] = 1;
		}

		ret = send(ctrl, buf, sizeof(buf), 0);
		if (ret < 0) {
			perror("send");
			CLEANUP_AND_EXIT;
		}

		/* Close the connection */
		close(ctrl);
	}
}