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
#include "tst_netlink.h"

/**
 * Add a crypto algorithm to a session.
 *
 * @param ctx Initialized netlink context
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
int tst_crypto_add_alg(struct tst_netlink_context *ctx,
		       const struct crypto_user_alg *alg);

/**
 * Delete a crypto algorithm from a session.
 *
 * @param ctx Initialized netlink context
 * @param alg The crypto algorithm to delete.
 * @param retries Number of retries before giving up. Recommended value: 1000
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
int tst_crypto_del_alg(struct tst_netlink_context *ctx,
	const struct crypto_user_alg *alg, unsigned int retries);

#endif	/* TST_CRYPTO_H */
