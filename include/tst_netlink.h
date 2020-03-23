/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
 */

/**
 * @file tst_netlink.h
 *
 * Library for communicating with the kernel over the netlink interface.
 */

#ifndef TST_NETLINK_H
#define TST_NETLINK_H

#include <linux/netlink.h>

#ifndef NETLINK_CRYPTO
/**
 * The netlink-crypto socket protocol.
 */
#define NETLINK_CRYPTO 21
#endif

/** @private */
static inline ssize_t safe_netlink_send(const char *file, const int lineno,
					int fd, const struct nlmsghdr *nh,
					const void *payload)
{
	struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
	struct iovec iov[2] = {
		{(struct nlmsghdr *)nh, sizeof(*nh)},
		{(void *)payload, nh->nlmsg_len - sizeof(*nh)}
	};
	struct msghdr msg = {
		.msg_name = &sa,
		.msg_namelen = sizeof(sa),
		.msg_iov = iov,
		.msg_iovlen = 2
	};

	return safe_sendmsg(file, lineno, nh->nlmsg_len, fd, &msg, 0);
}

/**
 * Sends a netlink message using safe_sendmsg().
 *
 * @param fd netlink socket file descriptor.
 * @param nl_header netlink header structure describing the message.
 * @param payload an opaque object containing the message data.
 *
 * You should set the message length, type and flags to appropriate values
 * within the nl_header object. See lib/tst_crypto.c for an example.
 *
 * @return The number of bytes sent.
 */
#define SAFE_NETLINK_SEND(fd, nl_header, payload)		\
	safe_netlink_send(__FILE__, __LINE__, fd, nl_header, payload)

/** @private */
static inline ssize_t safe_netlink_recv(const char *file, const int lineno,
					int fd, char *nl_headers_buf,
					size_t buf_len)
{
	struct iovec iov = { nl_headers_buf, buf_len };
	struct sockaddr_nl sa;
	struct msghdr msg = {
		.msg_name = &sa,
		.msg_namelen = sizeof(sa),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	return safe_recvmsg(file, lineno, 0, fd, &msg, 0);
}

/**
 * Receives a netlink message using safe_recvmsg().
 *
 * @param fd netlink socket file descriptor.
 * @param nl_header_buf buffer to contain the received netlink header structure.
 * @param buf_len The length of the header buffer. Must be greater than the page
 *                size.
 *
 * @return The number of bytes received.
 */
#define SAFE_NETLINK_RECV(fd, nl_header_buf, buf_len)			\
	safe_netlink_recv(__FILE__, __LINE__, fd, nl_header_buf, buf_len)

#endif /* TST_NETLINK_H */
