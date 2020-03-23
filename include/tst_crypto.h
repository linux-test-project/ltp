/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
 */

/**
 * @file tst_crypto.h
 *
 * Library for interacting with kernel's crypto layer using the netlink
 * interface.
 */

#ifndef TST_CRYPTO_H
#define TST_CRYPTO_H

#include "lapi/cryptouser.h"

/**
 * A reference to a crypto session and associated state.
 *
 * Holds state relevant to a netlink crypto connection. The seq_num is used
 * to tag each message sent to the netlink layer and is automatically
 * incremented by the tst_crypto_ functions. When the netlink layer sends a
 * response (ack) it will use the sequences number from the request.
 *
 * Some functions, such as delete ALG, may return EBUSY in which case it is
 * safe to retry them. The retries field allows you to set the number of
 * times this should be done. If set to zero the operation will only be tried
 * once. For operations which do not return EBUSY, the field is ignored.
 *
 * Use TST_CRYPTO_SESSION_INIT to statically initialize this struct with sane
 * defaults.
 */
struct tst_crypto_session {
	/** File descriptor for the netlink socket */
	int fd;
	/** A sequence number used to identify responses from the kernel. */
	uint32_t seq_num;
	/** Number of times some operations will be retried. */
	uint32_t retries;
};

/**
 * Default static definition of tst_crypto_session.
 *
 * @relates tst_crypto_session
 */
#define TST_CRYPTO_SESSION_INIT {\
	.fd = 0,                 \
	.seq_num = 0,            \
	.retries = 1000          \
}

/**
 * Creates a crypto session.
 *
 * @relates tst_crypto_session
 * @param ses Session structure to use, it can be uninitialized.
 *
 * If some necessary feature is missing then it will call tst_brk() with
 * TCONF, for any other error it will use TBROK.
 */
void tst_crypto_open(struct tst_crypto_session *ses);

/**
 * Close a crypto session.
 *
 * @relates tst_crypto_session
 * @param ses The session to close.
 */
void tst_crypto_close(struct tst_crypto_session *ses);

/**
 * Add a crypto algorithm to a session.
 *
 * @relates tst_crypto_session
 * @param ses An open session.
 * @param alg The crypto algorithm or module to add.
 *
 * This requests a new crypto algorithm/engine/module to be initialized by the
 * kernel. It sends the request contained in alg and then waits for a
 * response. If sending the message or receiving the ack fails at the netlink
 * level then tst_brk() with TBROK will be called.
 *
 * @return On success it will return 0 otherwise it will return an inverted
 *         error code from the crypto layer.
 */
int tst_crypto_add_alg(struct tst_crypto_session *ses,
		       const struct crypto_user_alg *alg);

/**
 * Delete a crypto algorithm from a session.
 *
 * @relates tst_crypto_session
 * @param ses An open session.
 * @param alg The crypto algorithm to delete.
 *
 * Request that the kernel remove an existing crypto algorithm. This behaves
 * in a similar way to tst_crypto_add_alg() except that it is the inverse
 * operation and that it is not unusual for the crypto layer to return
 * EBUSY. If EBUSY is returned then the function will internally retry the
 * operation tst_crypto_session::retries times before giving up and returning
 * EBUSY.
 *
 * Return: Either 0 or an inverted error code from the crypto layer. If called
 *         during cleanup it may return a positive ENODATA value from the LTP
 *         library, you don't need to log this error as it will already have
 *         been printed by tst_brk().
 */
int tst_crypto_del_alg(struct tst_crypto_session *ses,
		       const struct crypto_user_alg *alg);

#endif	/* TST_CRYPTO_H */
