// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
 *                    Nicolai Stange <nstange@suse.de>
 */

#include <errno.h>
#include <stdio.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_crypto.h"

int tst_crypto_add_alg(struct tst_netlink_context *ctx,
		       const struct crypto_user_alg *alg)
{
	struct nlmsghdr nh = {
		.nlmsg_type = CRYPTO_MSG_NEWALG,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
	};

	NETLINK_ADD_MESSAGE(ctx, &nh, alg, sizeof(struct crypto_user_alg));
	return NETLINK_SEND_VALIDATE(ctx) ? 0 : -tst_netlink_errno;
}

int tst_crypto_del_alg(struct tst_netlink_context *ctx,
	const struct crypto_user_alg *alg, unsigned int retries)
{
	int ret;
	unsigned int i = 0;
	struct nlmsghdr nh = {
		.nlmsg_type = CRYPTO_MSG_DELALG,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
	};

	for (i = 0; i < retries; i++) {
		NETLINK_ADD_MESSAGE(ctx, &nh, alg,
			sizeof(struct crypto_user_alg));

		if (NETLINK_SEND_VALIDATE(ctx))
			return 0;

		ret = -tst_netlink_errno;

		if (ret != -EBUSY)
			break;

		usleep(1);
	}

	return ret;
}
