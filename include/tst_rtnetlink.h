/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2021 Linux Test Project
 */

#ifndef TST_RTNETLINK_H
#define TST_RTNETLINK_H

struct tst_rtnl_context;

struct tst_rtnl_attr_list {
	unsigned short type;
	const void *data;
	ssize_t len;
	const struct tst_rtnl_attr_list *sublist;
};

struct tst_rtnl_message {
	struct nlmsghdr *header;
	struct nlmsgerr *err;
	void *payload;
	size_t payload_size;
};

/* Open a netlink socket */
struct tst_rtnl_context *tst_rtnl_create_context(const char *file,
	const int lineno);
#define RTNL_CREATE_CONTEXT() tst_rtnl_create_context(__FILE__, __LINE__)

/* Free a tst_rtnl_message array returned by tst_rtnl_recv() */
void tst_rtnl_free_message(struct tst_rtnl_message *msg);
#define RTNL_FREE_MESSAGE tst_rtnl_free_message

/* Close netlink socket */
void tst_rtnl_destroy_context(const char *file, const int lineno,
	struct tst_rtnl_context *ctx);
#define RTNL_DESTROY_CONTEXT(ctx) \
	tst_rtnl_destroy_context(__FILE__, __LINE__, (ctx))

/* Send all messages in given buffer */
int tst_rtnl_send(const char *file, const int lineno,
	struct tst_rtnl_context *ctx);
#define RTNL_SEND(ctx) tst_rtnl_send(__FILE__, __LINE__, (ctx))

/* Send all messages in given buffer and validate kernel response */
int tst_rtnl_send_validate(const char *file, const int lineno,
	struct tst_rtnl_context *ctx);
#define RTNL_SEND_VALIDATE(ctx) \
	tst_rtnl_send_validate(__FILE__, __LINE__, (ctx))

/* Wait until data is available for reading from the netlink socket */
int tst_rtnl_wait(struct tst_rtnl_context *ctx);
#define RTNL_WAIT tst_rtnl_wait

/*
 * Read from netlink socket and return an array of partially parsed messages.
 * header == NULL indicates end of array.
 */
struct tst_rtnl_message *tst_rtnl_recv(const char *file, const int lineno,
	struct tst_rtnl_context *ctx);
#define RTNL_RECV(ctx) tst_rtnl_recv(__FILE__, __LINE__, (ctx))

/* Add new message to buffer */
int tst_rtnl_add_message(const char *file, const int lineno,
	struct tst_rtnl_context *ctx, const struct nlmsghdr *header,
	const void *payload, size_t payload_size);
#define RTNL_ADD_MESSAGE(ctx, header, payload, psize) \
	tst_rtnl_add_message(__FILE__, __LINE__, (ctx), (header), (payload), \
		(psize))

/* Add arbitrary attribute to last message */
int tst_rtnl_add_attr(const char *file, const int lineno,
	struct tst_rtnl_context *ctx, unsigned short type, const void *data,
	unsigned short len);
#define RTNL_ADD_ATTR(ctx, type, data, len) \
	tst_rtnl_add_attr(__FILE__, __LINE__, (ctx), (type), (data), (len))

/* Add string attribute to last message */
int tst_rtnl_add_attr_string(const char *file, const int lineno,
	struct tst_rtnl_context *ctx, unsigned short type, const char *data);
#define RTNL_ADD_ATTR_STRING(ctx, type, data) \
	tst_rtnl_add_attr_string(__FILE__, __LINE__, (ctx), (type), (data))

/*
 * Add list of arbitrary attributes to last message. The list is terminated
 * by attribute with negative length. Nested sublists are supported.
 */
int tst_rtnl_add_attr_list(const char *file, const int lineno,
	struct tst_rtnl_context *ctx, const struct tst_rtnl_attr_list *list);
#define RTNL_ADD_ATTR_LIST(ctx, list) \
	tst_rtnl_add_attr_list(__FILE__, __LINE__, (ctx), (list))

/* Check that all sent messages with NLM_F_ACK flag have been acked without
 * error. Usage:
 *
 * tst_rtnl_send(ctx);
 * tst_rtnl_wait(ctx);
 * response = tst_rtnl_recv(ctx);
 * if (!tst_rtnl_check_acks(ctx, response)) { ... }
 * tst_rtnl_free_message(response);
 */
int tst_rtnl_check_acks(const char *file, const int lineno,
	struct tst_rtnl_context *ctx, struct tst_rtnl_message *response);
#define RTNL_CHECK_ACKS(ctx, response) \
	tst_rtnl_context(__FILE__, __LINE__, (ctx), (response))

#endif /* TST_RTNETLINK_H */
