/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2021 Linux Test Project
 */

#ifndef TST_NETLINK_H
#define TST_NETLINK_H

#include <linux/netlink.h>

struct tst_netlink_context;

struct tst_rtnl_attr_list {
	unsigned short type;
	const void *data;
	ssize_t len;
	const struct tst_rtnl_attr_list *sublist;
};

struct tst_netlink_message {
	struct nlmsghdr *header;
	struct nlmsgerr *err;
	void *payload;
	size_t payload_size;
};

extern int tst_netlink_errno;

/* Open a netlink socket */
struct tst_netlink_context *tst_netlink_create_context(const char *file,
	const int lineno, int protocol);
#define NETLINK_CREATE_CONTEXT(protocol) \
	tst_netlink_create_context(__FILE__, __LINE__, (protocol))

/* Free a tst_netlink_message array returned by tst_netlink_recv() */
void tst_netlink_free_message(struct tst_netlink_message *msg);
#define NETLINK_FREE_MESSAGE tst_netlink_free_message

/* Close netlink socket */
void tst_netlink_destroy_context(const char *file, const int lineno,
	struct tst_netlink_context *ctx);
#define NETLINK_DESTROY_CONTEXT(ctx) \
	tst_netlink_destroy_context(__FILE__, __LINE__, (ctx))

/* Send all messages in given buffer */
int tst_netlink_send(const char *file, const int lineno,
	struct tst_netlink_context *ctx);
#define NETLINK_SEND(ctx) tst_netlink_send(__FILE__, __LINE__, (ctx))

/* Send all messages in given buffer and validate kernel response */
int tst_netlink_send_validate(const char *file, const int lineno,
	struct tst_netlink_context *ctx);
#define NETLINK_SEND_VALIDATE(ctx) \
	tst_netlink_send_validate(__FILE__, __LINE__, (ctx))

/* Wait until data is available for reading from the netlink socket */
int tst_netlink_wait(struct tst_netlink_context *ctx);
#define NETLINK_WAIT tst_netlink_wait

/*
 * Read from netlink socket and return an array of partially parsed messages.
 * header == NULL indicates end of array.
 */
struct tst_netlink_message *tst_netlink_recv(const char *file, const int lineno,
	struct tst_netlink_context *ctx);
#define NETLINK_RECV(ctx) tst_netlink_recv(__FILE__, __LINE__, (ctx))

/* Add new message to buffer */
int tst_netlink_add_message(const char *file, const int lineno,
	struct tst_netlink_context *ctx, const struct nlmsghdr *header,
	const void *payload, size_t payload_size);
#define NETLINK_ADD_MESSAGE(ctx, header, payload, psize) \
	tst_netlink_add_message(__FILE__, __LINE__, (ctx), (header), \
		(payload), (psize))

/* Add arbitrary attribute to last message */
int tst_rtnl_add_attr(const char *file, const int lineno,
	struct tst_netlink_context *ctx, unsigned short type, const void *data,
	unsigned short len);
#define RTNL_ADD_ATTR(ctx, type, data, len) \
	tst_rtnl_add_attr(__FILE__, __LINE__, (ctx), (type), (data), (len))

/* Add string attribute to last message */
int tst_rtnl_add_attr_string(const char *file, const int lineno,
	struct tst_netlink_context *ctx, unsigned short type, const char *data);
#define RTNL_ADD_ATTR_STRING(ctx, type, data) \
	tst_rtnl_add_attr_string(__FILE__, __LINE__, (ctx), (type), (data))

/*
 * Add list of arbitrary attributes to last message. The list is terminated
 * by attribute with negative length. Nested sublists are supported.
 */
int tst_rtnl_add_attr_list(const char *file, const int lineno,
	struct tst_netlink_context *ctx, const struct tst_rtnl_attr_list *list);
#define RTNL_ADD_ATTR_LIST(ctx, list) \
	tst_rtnl_add_attr_list(__FILE__, __LINE__, (ctx), (list))

/* Check that all sent messages with NLM_F_ACK flag have been acked without
 * error. Usage:
 *
 * tst_netlink_send(ctx);
 * tst_netlink_wait(ctx);
 * response = tst_netlink_recv(ctx);
 * if (!tst_netlink_check_acks(ctx, response)) { ... }
 * tst_netlink_free_message(response);
 */
int tst_netlink_check_acks(const char *file, const int lineno,
	struct tst_netlink_context *ctx, struct tst_netlink_message *response);
#define NETLINK_CHECK_ACKS(ctx, response) \
	tst_netlink_check_acks(__FILE__, __LINE__, (ctx), (response))

#endif /* TST_NETLINK_H */
