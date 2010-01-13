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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	struct sockaddr_un sun;
	char buf[1024];
	int s, sunlen, ret, buflen;
	struct msghdr msg = { 0 };
	struct iovec iov;
	struct cmsghdr *cmsg;
	int myfd = 0;
	char cmsgbuf[CMSG_SPACE(sizeof myfd)];
	int *fdptr;

	if (argc != 3) {
		fprintf(stderr, "usage:  %s testfile address\n", argv[0]);
		exit(1);
	}

	myfd = open(argv[1], O_RDWR);
	if (myfd < 0) {
		perror(argv[1]);
		exit(1);
	}

	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		exit(1);
	}

	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, argv[2]);
	sunlen = strlen(sun.sun_path) + 1 + sizeof(short);
	ret = connect(s, (struct sockaddr *)&sun, sunlen);
	if (ret < 0) {
		perror("connect");
		exit(1);
	}

	printf("client: Connected to server via %s\n", sun.sun_path);

	strcpy(buf, "hello world");
	buflen = strlen(buf)+1;
	iov.iov_base = buf;
	iov.iov_len = buflen;
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof cmsgbuf;
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	fdptr = (int *)CMSG_DATA(cmsg);
	memcpy(fdptr, &myfd, sizeof(int));
	msg.msg_controllen = cmsg->cmsg_len;

	ret = sendmsg(s, &msg, 0);
	if (ret < 0) {
		perror("sendmsg");
		exit(1);
	}
	printf("client: Sent descriptor, waiting for reply\n");

	buf[0] = 0;
	ret = recv(s, buf, sizeof(buf), 0);
	if (ret < 0) {
		perror("recv");
		exit(1);
	}
	printf("client: Received reply, code=%d\n", buf[0]);
	if (buf[0])
		printf("client: ...This implies the descriptor was not received\n");
	else
		printf("client: ...This implies the descriptor was received\n");
	
	exit(buf[0]);
}
