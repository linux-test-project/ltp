// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Linux Test Project
 */

#include <stdlib.h>
#include <limits.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_netlink.h"

struct tst_netlink_context {
	int socket;
	pid_t pid;
	uint32_t seq;
	size_t bufsize, datalen;
	char *buffer;
	struct nlmsghdr *curmsg;
};

int tst_netlink_errno;

static int netlink_grow_buffer(const char *file, const int lineno,
	struct tst_netlink_context *ctx, size_t size)
{
	size_t needed, offset, curlen = NLMSG_ALIGN(ctx->datalen);
	char *buf;

	if (ctx->bufsize - curlen >= size)
		return 1;

	needed = size - (ctx->bufsize - curlen);
	size = ctx->bufsize + (ctx->bufsize > needed ? ctx->bufsize : needed);
	size = NLMSG_ALIGN(size);
	buf = safe_realloc(file, lineno, ctx->buffer, size);

	if (!buf)
		return 0;

	memset(buf + ctx->bufsize, 0, size - ctx->bufsize);
	offset = ((char *)ctx->curmsg) - ctx->buffer;
	ctx->buffer = buf;
	ctx->curmsg = (struct nlmsghdr *)(buf + offset);
	ctx->bufsize = size;

	return 1;
}

void tst_netlink_destroy_context(const char *file, const int lineno,
	struct tst_netlink_context *ctx)
{
	if (!ctx)
		return;

	safe_close(file, lineno, NULL, ctx->socket);
	free(ctx->buffer);
	free(ctx);
}

struct tst_netlink_context *tst_netlink_create_context(const char *file,
	const int lineno, int protocol)
{
	struct tst_netlink_context *ctx;
	struct sockaddr_nl addr = { .nl_family = AF_NETLINK };

	ctx = safe_malloc(file, lineno, NULL,
		sizeof(struct tst_netlink_context));

	if (!ctx)
		return NULL;

	ctx->pid = 0;
	ctx->seq = 0;
	ctx->buffer = NULL;
	ctx->bufsize = 1024;
	ctx->datalen = 0;
	ctx->curmsg = NULL;
	ctx->socket = safe_socket(file, lineno, NULL, AF_NETLINK,
		SOCK_DGRAM | SOCK_CLOEXEC, protocol);

	if (ctx->socket < 0) {
		free(ctx);
		return NULL;
	}

	if (safe_bind(file, lineno, NULL, ctx->socket, (struct sockaddr *)&addr,
		sizeof(addr))) {
		tst_netlink_destroy_context(file, lineno, ctx);
		return NULL;
	}

	ctx->buffer = safe_malloc(file, lineno, NULL, ctx->bufsize);

	if (!ctx->buffer) {
		tst_netlink_destroy_context(file, lineno, ctx);
		return NULL;
	}

	memset(ctx->buffer, 0, ctx->bufsize);

	return ctx;
}

void tst_netlink_free_message(struct tst_netlink_message *msg)
{
	if (!msg)
		return;

	// all ptr->header and ptr->info pointers point to the same buffer
	// msg->header is the start of the buffer
	free(msg->header);
	free(msg);
}

int tst_netlink_send(const char *file, const int lineno,
	struct tst_netlink_context *ctx)
{
	int ret;
	struct sockaddr_nl addr = { .nl_family = AF_NETLINK };
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &addr,
		.msg_namelen = sizeof(addr),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	if (!ctx->curmsg) {
		tst_brk_(file, lineno, TBROK, "%s(): No message to send",
			__func__);
		return 0;
	}

	if (ctx->curmsg->nlmsg_flags & NLM_F_MULTI) {
		struct nlmsghdr eom = { .nlmsg_type = NLMSG_DONE };

		if (!tst_netlink_add_message(file, lineno, ctx, &eom, NULL, 0))
			return 0;

		/* NLMSG_DONE message must not have NLM_F_MULTI flag */
		ctx->curmsg->nlmsg_flags = 0;
	}

	iov.iov_base = ctx->buffer;
	iov.iov_len = ctx->datalen;
	ret = safe_sendmsg(file, lineno, ctx->datalen, ctx->socket, &msg, 0);

	if (ret > 0)
		ctx->curmsg = NULL;

	return ret;
}

int tst_netlink_wait(struct tst_netlink_context *ctx)
{
	struct pollfd fdinfo = {
		.fd = ctx->socket,
		.events = POLLIN
	};

	return poll(&fdinfo, 1, 1000);
}

struct tst_netlink_message *tst_netlink_recv(const char *file,
	const int lineno, struct tst_netlink_context *ctx)
{
	char tmp, *tmpbuf, *buffer = NULL;
	struct tst_netlink_message *ret;
	struct nlmsghdr *ptr;
	size_t retsize, bufsize = 0;
	ssize_t size;
	int i, size_left, msgcount;

	/* Each recv() call returns one message, read all pending messages */
	while (1) {
		errno = 0;
		size = recv(ctx->socket, &tmp, 1,
			MSG_DONTWAIT | MSG_PEEK | MSG_TRUNC);

		if (size < 0) {
			if (errno != EAGAIN) {
				tst_brk_(file, lineno, TBROK | TERRNO,
					"recv() failed");
			}

			break;
		}

		tmpbuf = safe_realloc(file, lineno, buffer, bufsize + size);

		if (!tmpbuf)
			break;

		buffer = tmpbuf;
		size = safe_recv(file, lineno, size, ctx->socket,
			buffer + bufsize, size, 0);

		if (size < 0)
			break;

		bufsize += size;
	}

	if (!bufsize) {
		free(buffer);
		return NULL;
	}

	ptr = (struct nlmsghdr *)buffer;
	size_left = bufsize;
	msgcount = 0;

	for (; size_left > 0 && NLMSG_OK(ptr, size_left); msgcount++)
		ptr = NLMSG_NEXT(ptr, size_left);

	retsize = (msgcount + 1) * sizeof(struct tst_netlink_message);
	ret = safe_malloc(file, lineno, NULL, retsize);

	if (!ret) {
		free(buffer);
		return NULL;
	}

	memset(ret, 0, retsize);
	ptr = (struct nlmsghdr *)buffer;
	size_left = bufsize;

	for (i = 0; i < msgcount; i++, ptr = NLMSG_NEXT(ptr, size_left)) {
		ret[i].header = ptr;
		ret[i].payload = NLMSG_DATA(ptr);
		ret[i].payload_size = NLMSG_PAYLOAD(ptr, 0);

		if (ptr->nlmsg_type == NLMSG_ERROR)
			ret[i].err = NLMSG_DATA(ptr);
	}

	return ret;
}

int tst_netlink_add_message(const char *file, const int lineno,
	struct tst_netlink_context *ctx, const struct nlmsghdr *header,
	const void *payload, size_t payload_size)
{
	size_t size;
	unsigned int extra_flags = 0;

	if (!netlink_grow_buffer(file, lineno, ctx, NLMSG_SPACE(payload_size)))
		return 0;

	if (!ctx->curmsg) {
		/*
		 * datalen may hold the size of last sent message for ACK
		 * checking, reset it back to 0 here
		 */
		ctx->datalen = 0;
		ctx->curmsg = (struct nlmsghdr *)ctx->buffer;
	} else {
		size = NLMSG_ALIGN(ctx->curmsg->nlmsg_len);

		extra_flags = NLM_F_MULTI;
		ctx->curmsg->nlmsg_flags |= extra_flags;
		ctx->curmsg = NLMSG_NEXT(ctx->curmsg, size);
		ctx->datalen = NLMSG_ALIGN(ctx->datalen);
	}

	*ctx->curmsg = *header;
	ctx->curmsg->nlmsg_len = NLMSG_LENGTH(payload_size);
	ctx->curmsg->nlmsg_flags |= extra_flags;
	ctx->curmsg->nlmsg_seq = ctx->seq++;
	ctx->curmsg->nlmsg_pid = ctx->pid;

	if (payload_size)
		memcpy(NLMSG_DATA(ctx->curmsg), payload, payload_size);

	ctx->datalen += ctx->curmsg->nlmsg_len;

	return 1;
}

int tst_rtnl_add_attr(const char *file, const int lineno,
	struct tst_netlink_context *ctx, unsigned short type,
	const void *data, unsigned short len)
{
	size_t size;
	struct rtattr *attr;

	if (!ctx->curmsg) {
		tst_brk_(file, lineno, TBROK,
			"%s(): No message to add attributes to", __func__);
		return 0;
	}

	if (!netlink_grow_buffer(file, lineno, ctx, RTA_SPACE(len)))
		return 0;

	size = NLMSG_ALIGN(ctx->curmsg->nlmsg_len);
	attr = (struct rtattr *)(((char *)ctx->curmsg) + size);
	attr->rta_type = type;
	attr->rta_len = RTA_LENGTH(len);
	memcpy(RTA_DATA(attr), data, len);
	ctx->curmsg->nlmsg_len = size + attr->rta_len;
	ctx->datalen = NLMSG_ALIGN(ctx->datalen) + attr->rta_len;

	return 1;
}

int tst_rtnl_add_attr_string(const char *file, const int lineno,
	struct tst_netlink_context *ctx, unsigned short type,
	const char *data)
{
	return tst_rtnl_add_attr(file, lineno, ctx, type, data,
		strlen(data) + 1);
}

int tst_rtnl_add_attr_list(const char *file, const int lineno,
	struct tst_netlink_context *ctx,
	const struct tst_rtnl_attr_list *list)
{
	int i, ret;
	size_t offset;

	for (i = 0; list[i].len >= 0; i++) {
		if (list[i].len > USHRT_MAX) {
			tst_brk_(file, lineno, TBROK,
				"%s(): Attribute value too long", __func__);
			return -1;
		}

		offset = NLMSG_ALIGN(ctx->datalen);
		ret = tst_rtnl_add_attr(file, lineno, ctx, list[i].type,
			list[i].data, list[i].len);

		if (!ret)
			return -1;

		if (list[i].sublist) {
			struct rtattr *attr;

			ret = tst_rtnl_add_attr_list(file, lineno, ctx,
				list[i].sublist);

			if (ret < 0)
				return ret;

			attr = (struct rtattr *)(ctx->buffer + offset);

			if (ctx->datalen - offset > USHRT_MAX) {
				tst_brk_(file, lineno, TBROK,
					"%s(): Sublist too long", __func__);
				return -1;
			}

			attr->rta_len = ctx->datalen - offset;
		}
	}

	return i;
}

int tst_netlink_check_acks(const char *file, const int lineno,
	struct tst_netlink_context *ctx, struct tst_netlink_message *res)
{
	struct nlmsghdr *msg = (struct nlmsghdr *)ctx->buffer;
	int size_left = ctx->datalen;

	for (; size_left > 0 && NLMSG_OK(msg, size_left);
		msg = NLMSG_NEXT(msg, size_left)) {

		if (!(msg->nlmsg_flags & NLM_F_ACK))
			continue;

		while (res->header && res->header->nlmsg_seq != msg->nlmsg_seq)
			res++;

		if (!res->err || res->header->nlmsg_seq != msg->nlmsg_seq) {
			tst_brk_(file, lineno, TBROK,
				"No ACK found for Netlink message %u",
				msg->nlmsg_seq);
			return 0;
		}

		if (res->err->error) {
			tst_netlink_errno = -res->err->error;
			return 0;
		}
	}

	return 1;
}

int tst_netlink_send_validate(const char *file, const int lineno,
	struct tst_netlink_context *ctx)
{
	struct tst_netlink_message *response;
	int ret;

	tst_netlink_errno = 0;

	if (tst_netlink_send(file, lineno, ctx) <= 0)
		return 0;

	tst_netlink_wait(ctx);
	response = tst_netlink_recv(file, lineno, ctx);

	if (!response)
		return 0;

	ret = tst_netlink_check_acks(file, lineno, ctx, response);
	tst_netlink_free_message(response);

	return ret;
}
